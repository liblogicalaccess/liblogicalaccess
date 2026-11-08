#include <logicalaccess/plugins/cards/nedapmobileaccess/nedapmobileaccesscommands.hpp>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <boost/crc.hpp>
#include <algorithm>
#include <cstdint>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <array>

namespace logicalaccess
{
namespace
{
constexpr size_t AES_KEY_LENGTH = 16;
constexpr size_t AES_BLOCK_LENGTH = 16;
constexpr size_t RANDOM_DATA_LENGTH = 16;
constexpr size_t AES_MAC_LENGTH = 8;
constexpr size_t MAX_NATIVE_COMMAND_FIELD_VALUE = 0xffffff;

ByteVector rotateLeft(const ByteVector &data)
{
    ByteVector rotated(data.begin() + 1, data.end());
    rotated.push_back(data.front());
    return rotated;
}

void assertNativeStatus(const ISO7816Response &response, unsigned char sw2,
                        const char *message)
{
    EXCEPTION_ASSERT_WITH_LOG(response.getSW1() == 0x91 && response.getSW2() == sw2,
                              LibLogicalAccessException, message);
}

uint32_t calculateDesfireCRC32(const ByteVector &data)
{
    boost::crc_32_type crc;
    crc.process_bytes(data.data(), data.size());
    return ~crc.checksum();
}

bool matchesReadCRC(const ByteVector &plaintext, size_t dataLength)
{
    ByteVector crcInput(plaintext.begin(), plaintext.begin() + dataLength);
    crcInput.push_back(0x00);
    const uint32_t expectedCRC = calculateDesfireCRC32(crcInput);
    const size_t crcOffset = dataLength;

    return plaintext[crcOffset] == static_cast<unsigned char>(expectedCRC & 0xff) &&
           plaintext[crcOffset + 1] ==
               static_cast<unsigned char>((expectedCRC >> 8) & 0xff) &&
           plaintext[crcOffset + 2] ==
               static_cast<unsigned char>((expectedCRC >> 16) & 0xff) &&
           plaintext[crcOffset + 3] ==
               static_cast<unsigned char>((expectedCRC >> 24) & 0xff);
}

ByteVector decryptReadResponse(const ByteVector &ciphertext, const ByteVector &sessionKey,
                               ByteVector &sessionIV, size_t requestedLength)
{
    EXCEPTION_ASSERT_WITH_LOG(!ciphertext.empty() &&
                                  ciphertext.size() % AES_BLOCK_LENGTH == 0,
                              LibLogicalAccessException,
                              "Invalid encrypted Nedap Mobile Access read response length.");

    const ByteVector plaintext = AESHelper::AESDecrypt(ciphertext, sessionKey, sessionIV);
    size_t dataLength = requestedLength;

    if (requestedLength != 0)
    {
        EXCEPTION_ASSERT_WITH_LOG(dataLength + 4 <= plaintext.size(),
                                  LibLogicalAccessException,
                                  "Encrypted Nedap Mobile Access response is too short.");
        const size_t expectedPadding =
            (AES_BLOCK_LENGTH - ((dataLength + 4) % AES_BLOCK_LENGTH)) %
            AES_BLOCK_LENGTH;
        EXCEPTION_ASSERT_WITH_LOG(
            plaintext.size() == dataLength + 4 + expectedPadding &&
                std::all_of(plaintext.begin() + dataLength + 4, plaintext.end(),
                            [](unsigned char byte) { return byte == 0x00; }),
            LibLogicalAccessException,
            "Invalid encrypted Nedap Mobile Access response padding.");
    }
    else
    {
        bool foundLength = false;
        for (size_t candidate = 0; candidate + 4 <= plaintext.size(); ++candidate)
        {
            const size_t paddingLength = plaintext.size() - candidate - 4;
            const size_t expectedPadding =
                (AES_BLOCK_LENGTH - ((candidate + 4) % AES_BLOCK_LENGTH)) %
                AES_BLOCK_LENGTH;

            if (paddingLength == expectedPadding &&
                std::all_of(plaintext.begin() + candidate + 4, plaintext.end(),
                            [](unsigned char byte) { return byte == 0x00; }) &&
                matchesReadCRC(plaintext, candidate))
            {
                dataLength = candidate;
                foundLength = true;
                break;
            }
        }

        EXCEPTION_ASSERT_WITH_LOG(foundLength, LibLogicalAccessException,
                                  "Encrypted Nedap Mobile Access response CRC is invalid.");
    }

    EXCEPTION_ASSERT_WITH_LOG(matchesReadCRC(plaintext, dataLength),
                              LibLogicalAccessException,
                              "Encrypted Nedap Mobile Access response CRC is invalid.");
    sessionIV = ByteVector(ciphertext.end() - AES_BLOCK_LENGTH, ciphertext.end());
    return ByteVector(plaintext.begin(), plaintext.begin() + dataLength);
}
}

NedapMobileAccessCommands::NedapMobileAccessCommands()
    : Commands(CMD_NEDAP_MOBILE_ACCESS)
{
}

void NedapMobileAccessCommands::authenticateAES(const ByteVector &diversificationKey,
                                                const ByteVector &uid, unsigned int aid,
                                                const ByteVector &randomData,
                                                unsigned char keyNumber)
{
    clearAuthenticationState();
    EXCEPTION_ASSERT_WITH_LOG(diversificationKey.size() == AES_KEY_LENGTH,
                              LibLogicalAccessException,
                              "Nedap Mobile Access diversification key must be 16 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(!uid.empty(), LibLogicalAccessException,
                              "Nedap Mobile Access diversification requires a card UID.");
    EXCEPTION_ASSERT_WITH_LOG(aid <= MAX_NATIVE_COMMAND_FIELD_VALUE,
                              LibLogicalAccessException,
                              "Nedap Mobile Access diversification AID must be 3 bytes.");
    EXCEPTION_ASSERT_WITH_LOG(randomData.size() == RANDOM_DATA_LENGTH,
                              LibLogicalAccessException,
                              "Nedap Mobile Access random data must be 16 bytes.");

    ByteVector diversificationInput = {0x01};
    diversificationInput.insert(diversificationInput.end(), uid.begin(), uid.end());
    diversificationInput.push_back(static_cast<unsigned char>((aid >> 16) & 0xff));
    diversificationInput.push_back(static_cast<unsigned char>((aid >> 8) & 0xff));
    diversificationInput.push_back(static_cast<unsigned char>(aid & 0xff));
    diversificationInput.insert(diversificationInput.end(), randomData.begin(),
                                randomData.end());

    const ByteVector diversifiedKey =
        openssl::CMACCrypto::cmac(diversificationKey, "aes", diversificationInput, {}, 32);
    authenticateAES(diversifiedKey, keyNumber);
}

void NedapMobileAccessCommands::authenticateAES(const ByteVector &diversifiedKey,
                                                unsigned char keyNumber)
{
    clearAuthenticationState();
    EXCEPTION_ASSERT_WITH_LOG(diversifiedKey.size() == AES_KEY_LENGTH,
                              LibLogicalAccessException,
                              "Nedap Mobile Access AES key must be 16 bytes.");

    auto response = sendNativeCommand(0xAA, {keyNumber});
    assertNativeStatus(response, 0xAF,
                       "Nedap Mobile Access AES authentication did not request another frame.");
    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() == AES_BLOCK_LENGTH,
                              LibLogicalAccessException,
                              "Invalid encrypted challenge length from Nedap Mobile Access.");

    const ByteVector encryptedRndB = response.getData();
    const ByteVector rndB = AESHelper::AESDecrypt(encryptedRndB, diversifiedKey, {});
    EXCEPTION_ASSERT_WITH_LOG(rndB.size() == AES_BLOCK_LENGTH,
                              LibLogicalAccessException,
                              "Invalid decrypted challenge from Nedap Mobile Access.");

    std::array<unsigned char, AES_BLOCK_LENGTH> rndA{};
    EXCEPTION_ASSERT_WITH_LOG(RAND_bytes(rndA.data(), static_cast<int>(rndA.size())) == 1,
                              LibLogicalAccessException,
                              "Cannot retrieve cryptographically strong random bytes.");

    ByteVector challenge(rndA.begin(), rndA.end());
    const ByteVector rotatedRndB = rotateLeft(rndB);
    challenge.insert(challenge.end(), rotatedRndB.begin(), rotatedRndB.end());
    const ByteVector encryptedChallenge =
        AESHelper::AESEncrypt(challenge, diversifiedKey, encryptedRndB);

    response = sendNativeCommand(0xAF, encryptedChallenge);
    assertNativeStatus(response, 0x00,
                       "Nedap Mobile Access AES authentication failed.");
    EXCEPTION_ASSERT_WITH_LOG(response.getData().size() == AES_BLOCK_LENGTH,
                              LibLogicalAccessException,
                              "Invalid authentication response from Nedap Mobile Access.");

    const ByteVector rotatedRndA =
        AESHelper::AESDecrypt(
            response.getData(), diversifiedKey,
            ByteVector(encryptedChallenge.end() - AES_BLOCK_LENGTH,
                       encryptedChallenge.end()));
    EXCEPTION_ASSERT_WITH_LOG(
        rotatedRndA == rotateLeft(ByteVector(rndA.begin(), rndA.end())),
        LibLogicalAccessException,
        "Nedap Mobile Access AES mutual authentication failed.");

    const ByteVector rndAVector(rndA.begin(), rndA.end());
    sessionKey_.insert(sessionKey_.end(), rndAVector.begin(), rndAVector.begin() + 4);
    sessionKey_.insert(sessionKey_.end(), rndB.begin(), rndB.begin() + 4);
    sessionKey_.insert(sessionKey_.end(), rndAVector.end() - 4, rndAVector.end());
    sessionKey_.insert(sessionKey_.end(), rndB.end() - 4, rndB.end());
    sessionIV_.assign(AES_BLOCK_LENGTH, 0x00);
}

ISO7816Response NedapMobileAccessCommands::readData(unsigned char fileNumber, size_t offset,
                                                    size_t length,
                                                    NedapMobileAccessCommunicationMode
                                                        communicationMode)
{
    EXCEPTION_ASSERT_WITH_LOG(isAuthenticated(), LibLogicalAccessException,
                              "Nedap Mobile Access data read requires AES authentication.");
    EXCEPTION_ASSERT_WITH_LOG(offset <= MAX_NATIVE_COMMAND_FIELD_VALUE &&
                                  length <= MAX_NATIVE_COMMAND_FIELD_VALUE,
                              LibLogicalAccessException,
                              "Nedap Mobile Access read offset or length is too large.");

    ByteVector data = {
        fileNumber,
        static_cast<unsigned char>(offset & 0xff),
        static_cast<unsigned char>((offset >> 8) & 0xff),
        static_cast<unsigned char>((offset >> 16) & 0xff),
        static_cast<unsigned char>(length & 0xff),
        static_cast<unsigned char>((length >> 8) & 0xff),
        static_cast<unsigned char>((length >> 16) & 0xff)};
    ISO7816Response response = sendNativeCommand(0xBD, data);
    ByteVector responseData = response.getData();
    while (response.getSW1() == 0x91 && response.getSW2() == 0xAF)
    {
        response = sendNativeCommand(0xAF, {});
        responseData.insert(responseData.end(), response.getData().begin(),
                            response.getData().end());
    }
    EXCEPTION_ASSERT_WITH_LOG(response.getSW1() == 0x91 && response.getSW2() == 0x00,
                              LibLogicalAccessException,
                              "Nedap Mobile Access data read failed.");

    switch (communicationMode)
    {
    case NedapMobileAccessCommunicationMode::Plain:
    case NedapMobileAccessCommunicationMode::Mac:
    {
        EXCEPTION_ASSERT_WITH_LOG(responseData.size() >= AES_MAC_LENGTH,
                                  LibLogicalAccessException,
                                  "Nedap Mobile Access MAC response is too short.");
        const size_t dataLength = responseData.size() - AES_MAC_LENGTH;
        ByteVector macInput(responseData.begin(), responseData.begin() + dataLength);
        macInput.push_back(0x00);
        const ByteVector calculatedMac = openssl::CMACCrypto::cmac(
            sessionKey_, "aes", macInput, sessionIV_, AES_BLOCK_LENGTH);
        EXCEPTION_ASSERT_WITH_LOG(
            CRYPTO_memcmp(responseData.data() + dataLength, calculatedMac.data(),
                          AES_MAC_LENGTH) == 0,
            LibLogicalAccessException,
            "Nedap Mobile Access read response MAC verification failed.");
        responseData.resize(dataLength);
        sessionIV_ = calculatedMac;
        break;
    }
    case NedapMobileAccessCommunicationMode::Enciphered:
        responseData =
            decryptReadResponse(responseData, sessionKey_, sessionIV_, length);
        break;
    default:
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Unknown Nedap Mobile Access communication mode.");
    }

    return ISO7816Response(responseData, response.getSW1(), response.getSW2());
}

ISO7816Response NedapMobileAccessCommands::requestUserAuthentication(
    const ByteVector &authenticationRequest)
{
    EXCEPTION_ASSERT_WITH_LOG(isAuthenticated(), LibLogicalAccessException,
                              "User authentication requires Nedap Mobile Access AES authentication.");
    return sendNativeCommand(0xEA, authenticationRequest);
}

ISO7816Response NedapMobileAccessCommands::endTransaction()
{
    auto response = sendNativeCommand(0x5A, {0x00, 0x00, 0x00});
    assertNativeStatus(response, 0x00,
                       "Nedap Mobile Access could not end the transaction.");
    clearAuthenticationState();
    return response;
}
}
