#include <gtest/gtest.h>

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpav2keydiversification.hpp>
#include <logicalaccess/plugins/cards/duox/duoxchip.hpp>
#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev3iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>

using namespace logicalaccess;

namespace
{

ByteVector sequence(std::size_t size, std::uint8_t start = 0)
{
    ByteVector data(size);
    for (std::size_t i = 0; i < size; ++i)
        data[i] = static_cast<std::uint8_t>(start + i);
    return data;
}

std::shared_ptr<DESFireKey> makeKey(const ByteVector &data)
{
    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(data.size() == 32 ? DF_KEY_AES256 : DF_KEY_AES);
    key->setData(data);
    return key;
}

ByteVector rotate(ByteVector data)
{
    std::rotate(data.begin(), data.begin() + 1, data.end());
    return data;
}

enum class Fault
{
    None, Part1SW1, Part1SW2, Part1Short, Part1Long,
    Part2SW1, Part2SW2, Part2Short, Part2Long,
    Nonce, Capabilities, Transport
};

template <typename Base>
class TestCrypto : public Base
{
  public:
    using Base::d_buf;
    using Base::d_last_left;
    using Base::d_rndA;
    using Base::d_rndB;
};

class AuthenticationCard
{
  public:
    ByteVector key = sequence(16);
    ByteVector rndB = sequence(16, 0x10);
    ByteVector rndA;
    ByteVector ti = {1, 2, 3, 4};
    ByteVector capabilities;
    ByteVector part1Command;
    Fault fault = Fault::None;
    unsigned int exchanges = 0;
    bool nonFirst = false;

    ISO7816Response exchange(std::uint8_t cmd, const ByteVector &data)
    {
        ++exchanges;
        if (fault == Fault::Transport)
            throw std::runtime_error("Injected transport failure");
        if (exchanges == 1)
        {
            EXPECT_EQ(cmd, nonFirst ? 0x77 : 0x71);
            part1Command = data;
            if (!nonFirst)
            {
                EXCEPTION_ASSERT_WITH_LOG(data.size() >= 2, LibLogicalAccessException,
                                          "Invalid test authentication command.");
                EXPECT_EQ(data[1], data.size() - 2);
                capabilities = ByteVector(data.begin() + 2, data.end());
            }
            ByteVector encrypted = AESHelper::AESEncrypt(rndB, key, {});
            if (fault == Fault::Part1Short)
                encrypted.resize(15);
            if (fault == Fault::Part1Long)
                encrypted.resize(32);
            return ISO7816Response(encrypted, fault == Fault::Part1SW1 ? 0x90 : 0x91,
                                   fault == Fault::Part1SW2 ? 0x00 : 0xAF);
        }
        EXPECT_EQ(exchanges, 2U);
        EXPECT_EQ(cmd, 0xAF);
        EXPECT_EQ(data.size(), 32U);
        const auto challenge = AESHelper::AESDecrypt(data, key, {});
        EXCEPTION_ASSERT_WITH_LOG(challenge.size() == 32, LibLogicalAccessException,
                                  "Invalid test authentication challenge.");
        rndA = ByteVector(challenge.begin(), challenge.begin() + 16);
        EXPECT_EQ(ByteVector(challenge.begin() + 16, challenge.end()), rotate(rndB));
        auto rotatedA = rotate(rndA);
        if (fault == Fault::Nonce)
            rotatedA[0] ^= 1;
        ByteVector response = nonFirst ? ByteVector() : ti;
        response.insert(response.end(), rotatedA.begin(), rotatedA.end());
        if (!nonFirst)
        {
            const ByteVector piccCapabilities = {6, 5, 4, 3, 2, 1};
            response.insert(response.end(), piccCapabilities.begin(), piccCapabilities.end());
            auto echoed = capabilities;
            echoed.resize(6, 0);
            if (fault == Fault::Capabilities)
                echoed[0] ^= 1;
            response.insert(response.end(), echoed.begin(), echoed.end());
        }
        auto encrypted = AESHelper::AESEncrypt(response, key, {});
        if (fault == Fault::Part2Short)
            encrypted.pop_back();
        if (fault == Fault::Part2Long)
            encrypted.resize(encrypted.size() + 16);
        return ISO7816Response(encrypted, fault == Fault::Part2SW1 ? 0x90 : 0x91,
                               fault == Fault::Part2SW2 ? 0xAF : 0x00);
    }
};

template <typename Base>
class TestCommands : public Base
{
  public:
    explicit TestCommands(const std::shared_ptr<DESFireChip> &chip) : chip_(chip)
    {
        if (std::dynamic_pointer_cast<DUOXCrypto>(chip->getCrypto()))
            chip->setCrypto(std::make_shared<TestCrypto<DUOXCrypto>>());
        else
            chip->setCrypto(std::make_shared<TestCrypto<DESFireEV2Crypto>>());
        this->setChip(chip);
    }

    AuthenticationCard card;

  protected:
    ISO7816Response transmit_plain(unsigned char cmd, const ByteVector &data,
                                   unsigned char, bool) override
    {
        return card.exchange(cmd, data);
    }

  private:
    std::shared_ptr<DESFireChip> chip_;
};

void expectTI(DESFireEV2Crypto &crypto, const ByteVector &ti)
{
    const auto counter = crypto.getCmdCtr();
    ByteVector input = {0x60, static_cast<std::uint8_t>(counter & 0xFF),
                        static_cast<std::uint8_t>(counter >> 8)};
    input.insert(input.end(), ti.begin(), ti.end());
    const auto full = openssl::CMACCrypto::cmac(crypto.d_macSessionKey, "aes", input, {});
    ByteVector expected;
    for (std::size_t i = 1; i < full.size(); i += 2)
        expected.push_back(full[i]);
    EXPECT_EQ(crypto.generateMAC(0x60, {}), expected);
}

template <typename Base>
void seedSession(TestCrypto<Base> &crypto, CryptoMethod method)
{
    crypto.installFirstSession({ByteVector(16, 0x11), ByteVector(16, 0x22)},
                               {9, 8, 7, 6}, method, 1);
    crypto.setCmdCtr(0x1234);
    crypto.d_authkey = {1};
    crypto.d_rndA = {2};
    crypto.d_rndB = {3};
    crypto.d_lastIV = {4};
    crypto.d_last_left = {5};
    crypto.d_buf = {6};
}

template <typename Base>
void expectInvalidated(const TestCrypto<Base> &crypto)
{
    EXPECT_EQ(crypto.d_auth_method, CM_LEGACY);
    EXPECT_EQ(crypto.getCmdCtr(), 0);
    EXPECT_EQ(crypto.d_currentKeyNo, 0);
    EXPECT_TRUE(crypto.d_sessionKey.empty());
    EXPECT_TRUE(crypto.d_macSessionKey.empty());
    EXPECT_TRUE(crypto.d_authkey.empty());
    EXPECT_TRUE(crypto.d_rndA.empty());
    EXPECT_TRUE(crypto.d_rndB.empty());
    EXPECT_TRUE(crypto.d_lastIV.empty());
    EXPECT_TRUE(crypto.d_last_left.empty());
    EXPECT_TRUE(crypto.d_buf.empty());
    EXPECT_EQ(crypto.d_cipher, nullptr);
    EXPECT_EQ(crypto.d_mac_size, 4U);
}

template <typename CommandsType, typename ChipType>
void testDESFireFirst()
{
    const auto chip = std::make_shared<ChipType>();
    TestCommands<CommandsType> commands(chip);
    const auto crypto = std::dynamic_pointer_cast<TestCrypto<DESFireEV2Crypto>>(chip->getCrypto());
    seedSession(*crypto, CM_EV2);
    commands.authenticateEV2First(3, makeKey(commands.card.key));
    EXPECT_EQ(commands.card.part1Command, (ByteVector{3, 0}));
    EXPECT_EQ(crypto->d_auth_method, CM_EV2);
    EXPECT_EQ(crypto->d_currentKeyNo, 3);
    EXPECT_EQ(crypto->getCmdCtr(), 0);
    const auto expected = DESFireEV2Crypto::deriveEV2SessionKeys(
        commands.card.key, commands.card.rndA, commands.card.rndB);
    EXPECT_EQ(crypto->d_sessionKey, expected.first);
    EXPECT_EQ(crypto->d_macSessionKey, expected.second);
    expectTI(*crypto, commands.card.ti);
}

} // namespace

TEST(EV2AuthenticationCrypto, KnownAnswerAES128)
{
    // Independent OpenSSL CMAC values for RndA=00..0F, RndB=10..1F, key=00..0F.
    const ByteVector enc = {0xF4, 0xDA, 0x87, 0xF0, 0x88, 0x43, 0x6F, 0x16,
                            0xFB, 0xFE, 0xB1, 0xAE, 0x3D, 0x72, 0x6F, 0xCB};
    const ByteVector mac = {0x1F, 0x46, 0x3E, 0xD7, 0x52, 0xE1, 0x04, 0x32,
                            0x9C, 0xD7, 0x50, 0x79, 0x8B, 0x50, 0x0D, 0xA8};
    const auto keys = DESFireEV2Crypto::deriveEV2SessionKeys(sequence(16), sequence(16), sequence(16, 0x10));
    EXPECT_EQ(keys.first, enc);
    EXPECT_EQ(keys.second, mac);
    DUOXCrypto duox;
    EXPECT_EQ(duox.deriveSessionKeys(sequence(16), sequence(16), sequence(16, 0x10)), keys);
    DESFireEV2Crypto ev2;
    ev2.generateSessionKey(sequence(16), sequence(16), sequence(16, 0x10));
    EXPECT_EQ(ev2.d_sessionKey, enc);
    EXPECT_EQ(ev2.d_macSessionKey, mac);
}

TEST(EV2AuthenticationCrypto, KnownAnswerAES256)
{
    const ByteVector enc = {0xBC, 0x51, 0x86, 0x4A, 0xF0, 0xA4, 0xC8, 0x24,
        0xAB, 0xF7, 0xED, 0xEF, 0xD3, 0x9A, 0x02, 0x83,
        0xE1, 0x52, 0x37, 0x85, 0x27, 0x90, 0x09, 0xC4,
        0x9B, 0x5E, 0xAA, 0x67, 0xF9, 0x56, 0x09, 0x93};
    const ByteVector mac = {0x08, 0x44, 0x42, 0x12, 0xDA, 0x90, 0xE2, 0xC3,
        0xAF, 0xB3, 0x97, 0x1D, 0x22, 0xF3, 0xEB, 0x86,
        0xC0, 0xF8, 0xD7, 0x1D, 0x8F, 0x5D, 0xDF, 0x33,
        0x9A, 0x7C, 0xA9, 0xF8, 0x14, 0x51, 0x0F, 0xFD};
    TestCrypto<DUOXCrypto> crypto;
    seedSession(crypto, CM_DUOX);
    const auto keys = crypto.deriveSessionKeys(sequence(32), sequence(16), sequence(16, 0x10));
    EXPECT_EQ(keys.first, enc);
    EXPECT_EQ(keys.second, mac);
    EXPECT_EQ(crypto.getCmdCtr(), 0x1234);
    EXPECT_EQ(crypto.d_sessionKey, ByteVector(16, 0x11));
}

TEST(EV2AuthenticationCrypto, RejectsInvalidInputs)
{
    for (std::size_t size : {0U, 15U, 24U, 31U, 33U})
        EXPECT_THROW(DESFireEV2Crypto::deriveEV2SessionKeys(sequence(size), sequence(16), sequence(16)),
                     LibLogicalAccessException);
    for (std::size_t size : {0U, 15U, 17U})
    {
        EXPECT_THROW(DESFireEV2Crypto::rotateAuthenticationChallenge(sequence(size)), LibLogicalAccessException);
        EXPECT_THROW(DESFireEV2Crypto::deriveEV2SessionKeys(sequence(16), sequence(size), sequence(16)),
                     LibLogicalAccessException);
        EXPECT_THROW(DESFireEV2Crypto::deriveEV2SessionKeys(sequence(16), sequence(16), sequence(size)),
                     LibLogicalAccessException);
    }
}

TEST(EV2AuthenticationCrypto, ParsesCapabilitiesAndValidatesExactResponseShape)
{
    const auto rndA = sequence(16);
    ByteVector data = {1, 2, 3, 4};
    const auto rotated = rotate(rndA);
    data.insert(data.end(), rotated.begin(), rotated.end());
    const ByteVector picc = {6, 5, 4, 3, 2, 1};
    const ByteVector pcd = {1, 2, 3, 0, 0, 0};
    data.insert(data.end(), picc.begin(), picc.end());
    data.insert(data.end(), pcd.begin(), pcd.end());
    const auto parsed = DESFireEV2Crypto::parseEV2FirstResponse(data, rndA, {1, 2, 3}, true);
    EXPECT_EQ(parsed.ti, (ByteVector{1, 2, 3, 4}));
    EXPECT_EQ(parsed.piccCap2, picc);
    EXPECT_EQ(parsed.pcdCap2, pcd);
    EXPECT_EQ(DESFireEV2Crypto::auth_ev2_part2(data, rndA), parsed.ti);
    for (std::size_t size : {0U, 16U, 31U, 33U, 48U})
        EXPECT_THROW(DESFireEV2Crypto::parseEV2FirstResponse(ByteVector(size), rndA), LibLogicalAccessException);
    EXPECT_THROW(DESFireEV2Crypto::parseEV2FirstResponse(data, sequence(15)), LibLogicalAccessException);
    EXPECT_THROW(DESFireEV2Crypto::parseEV2FirstResponse(data, rndA, ByteVector(7), true),
                 LibLogicalAccessException);
}

TEST(EV2AuthenticationCommands, DESFireEV2AndEV3First)
{
    testDESFireFirst<DESFireEV2ISO7816Commands, DESFireEV2Chip>();
    testDESFireFirst<DESFireEV3ISO7816Commands, DESFireEV3Chip>();
}

TEST(EV2AuthenticationCommands, DESFireDefaultKeyAndAES256Restriction)
{
    const auto chip = std::make_shared<DESFireEV2Chip>();
    TestCommands<DESFireEV2ISO7816Commands> commands(chip);
    commands.card.key = ByteVector(16, 0);
    EXPECT_NO_THROW(commands.authenticateEV2First(0, nullptr));
    const auto crypto = std::dynamic_pointer_cast<TestCrypto<DESFireEV2Crypto>>(chip->getCrypto());
    crypto->setCmdCtr(7);
    EXPECT_THROW(commands.authenticateEV2First(0, makeKey(sequence(32))), LibLogicalAccessException);
    EXPECT_EQ(commands.card.exchanges, 2U);
    EXPECT_EQ(crypto->getCmdCtr(), 7);
}

TEST(EV2AuthenticationCommands, DUOXFirstKeySizesAndCapabilities)
{
    for (std::size_t size : {16U, 32U})
    {
        for (std::size_t capSize : {0U, 3U, 6U})
        {
            const auto chip = std::make_shared<DUOXChip>();
            TestCommands<DUOXISO7816Commands> commands(chip);
            commands.card.key = sequence(size);
            const auto crypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(chip->getCrypto());
            const auto caps = sequence(capSize, 1);
            commands.authenticateEV2First(3, makeKey(commands.card.key), true, caps);
            ByteVector expectedCommand = {0x83, static_cast<std::uint8_t>(capSize)};
            expectedCommand.insert(expectedCommand.end(), caps.begin(), caps.end());
            EXPECT_EQ(commands.card.part1Command, expectedCommand);
            EXPECT_EQ(crypto->d_auth_method, CM_DUOX);
            EXPECT_EQ(crypto->d_currentKeyNo, 3);
            EXPECT_EQ(crypto->getCmdCtr(), 0);
            EXPECT_EQ(crypto->d_sessionKey.size(), size);
            const auto expected = crypto->deriveSessionKeys(commands.card.key, commands.card.rndA, commands.card.rndB);
            EXPECT_EQ(crypto->d_sessionKey, expected.first);
            EXPECT_EQ(crypto->d_macSessionKey, expected.second);
            expectTI(*crypto, commands.card.ti);
            EXPECT_EQ(crypto->getKey(0, 3)->getKeyType(), size == 32 ? DF_KEY_AES256 : DF_KEY_AES);
        }
    }
}

TEST(EV2AuthenticationCommands, DUOXEncodedKeyNoAndInvalidPreflight)
{
    const auto chip = std::make_shared<DUOXChip>();
    TestCommands<DUOXISO7816Commands> commands(chip);
    const auto crypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(chip->getCrypto());
    seedSession(*crypto, CM_DUOX);
    EXPECT_THROW(commands.authenticateEV2First(0x40, makeKey(commands.card.key)), LibLogicalAccessException);
    EXPECT_THROW(commands.authenticateEV2First(3, nullptr), LibLogicalAccessException);
    EXPECT_THROW(commands.authenticateEV2First(3, makeKey(commands.card.key), false, ByteVector(7)),
                 LibLogicalAccessException);
    EXPECT_EQ(commands.card.exchanges, 0U);
    EXPECT_EQ(crypto->getCmdCtr(), 0x1234);
    commands.authenticateEV2First(0x83, makeKey(commands.card.key));
    EXPECT_EQ(commands.card.part1Command, (ByteVector{0x83, 0}));
    EXPECT_EQ(crypto->d_currentKeyNo, 3);
}

TEST(EV2AuthenticationCommands, FirstUsesDiversifiedKeyAndPreservesKeyOwnership)
{
    const ByteVector raw = {0xF3, 0xF9, 0x37, 0x76, 0x98, 0x70, 0x7B, 0x68,
                            0x8E, 0xAF, 0x84, 0xAB, 0xE3, 0x9E, 0x37, 0x91};
    const ByteVector input = {0x04, 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    const ByteVector diversified = {0x0B, 0xB4, 0x08, 0xBA, 0xFF, 0x98, 0xB6, 0xEE,
                                    0x9F, 0x2E, 0x15, 0x85, 0x77, 0x7F, 0x6A, 0x51};
    const auto key = makeKey(raw);
    key->setKeyDiversification(std::make_shared<NXPAV2KeyDiversification>(input));

    const auto ev2Chip = std::make_shared<DESFireEV2Chip>();
    TestCommands<DESFireEV2ISO7816Commands> ev2(ev2Chip);
    ev2.card.key = diversified;
    EXPECT_NO_THROW(ev2.authenticateEV2First(3, key));
    EXPECT_NE(ev2Chip->getCrypto()->getKey(0, 3), key);

    const auto duoxChip = std::make_shared<DUOXChip>();
    TestCommands<DUOXISO7816Commands> duox(duoxChip);
    duox.card.key = diversified;
    EXPECT_NO_THROW(duox.authenticateEV2First(3, key));
    EXPECT_EQ(duoxChip->getCrypto()->getKey(0, 3), key);
}

TEST(EV2AuthenticationCommands, DiversificationPreflightFailurePreservesSession)
{
    const auto key = makeKey(sequence(16));
    key->setKeyDiversification(std::make_shared<NXPAV2KeyDiversification>());
    const auto chip = std::make_shared<DUOXChip>();
    TestCommands<DUOXISO7816Commands> commands(chip);
    const auto crypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(chip->getCrypto());
    seedSession(*crypto, CM_DUOX);
    EXPECT_THROW(commands.authenticateEV2First(3, key), LibLogicalAccessException);
    EXPECT_EQ(commands.card.exchanges, 0U);
    EXPECT_EQ(crypto->getCmdCtr(), 0x1234);
    EXPECT_EQ(crypto->d_auth_method, CM_DUOX);
}

TEST(EV2AuthenticationCommands, FirstFailureInvalidatesBothFamilies)
{
    for (Fault fault : {Fault::Part1SW1, Fault::Part1SW2, Fault::Part1Short, Fault::Part1Long,
                        Fault::Part2SW1, Fault::Part2SW2, Fault::Part2Short, Fault::Part2Long, Fault::Nonce})
    {
        const auto ev2Chip = std::make_shared<DESFireEV2Chip>();
        TestCommands<DESFireEV2ISO7816Commands> ev2(ev2Chip);
        const auto ev2Crypto = std::dynamic_pointer_cast<TestCrypto<DESFireEV2Crypto>>(ev2Chip->getCrypto());
        seedSession(*ev2Crypto, CM_EV2);
        ev2.card.fault = fault;
        EXPECT_THROW(ev2.authenticateEV2First(3, makeKey(ev2.card.key)), LibLogicalAccessException);
        expectInvalidated(*ev2Crypto);

        const auto duoxChip = std::make_shared<DUOXChip>();
        TestCommands<DUOXISO7816Commands> duox(duoxChip);
        const auto duoxCrypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(duoxChip->getCrypto());
        seedSession(*duoxCrypto, CM_DUOX);
        duox.card.fault = fault;
        EXPECT_THROW(duox.authenticateEV2First(3, makeKey(duox.card.key)), LibLogicalAccessException);
        expectInvalidated(*duoxCrypto);
    }
}

TEST(EV2AuthenticationCommands, CapabilityValidationRemainsFamilySpecific)
{
    const auto ev2Chip = std::make_shared<DESFireEV2Chip>();
    TestCommands<DESFireEV2ISO7816Commands> ev2(ev2Chip);
    ev2.card.fault = Fault::Capabilities;
    EXPECT_NO_THROW(ev2.authenticateEV2First(3, makeKey(ev2.card.key)));

    const auto duoxChip = std::make_shared<DUOXChip>();
    TestCommands<DUOXISO7816Commands> duox(duoxChip);
    duox.card.fault = Fault::Capabilities;
    EXPECT_THROW(duox.authenticateEV2First(3, makeKey(duox.card.key)), LibLogicalAccessException);
    expectInvalidated(*std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(duoxChip->getCrypto()));
}

TEST(EV2AuthenticationCommands, NonFirstPreservesTransactionStateAndDerivesBothKeySizes)
{
    for (CryptoMethod method : {CM_EV2, CM_DUOX})
    {
        for (std::size_t size : {16U, 32U})
        {
            const auto chip = std::make_shared<DUOXChip>();
            TestCommands<DUOXISO7816Commands> commands(chip);
            const auto crypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(chip->getCrypto());
            seedSession(*crypto, method);
            commands.card.nonFirst = true;
            commands.card.key = sequence(size);
            crypto->setKey(crypto->d_currentAid, 0, 3, makeKey(commands.card.key));
            commands.authenticateEV2NonFirst(3);
            EXPECT_EQ(commands.card.part1Command, (ByteVector{3}));
            EXPECT_EQ(crypto->getCmdCtr(), 0x1234);
            EXPECT_EQ(crypto->d_auth_method, method);
            EXPECT_EQ(crypto->d_currentKeyNo, 3);
            const auto keys = crypto->deriveSessionKeys(commands.card.key, commands.card.rndA, commands.card.rndB);
            EXPECT_EQ(crypto->d_sessionKey, keys.first);
            EXPECT_EQ(crypto->d_macSessionKey, keys.second);
            expectTI(*crypto, {9, 8, 7, 6});
        }
    }
}

TEST(EV2AuthenticationCommands, NonFirstFailureAndTransportFailureInvalidateSession)
{
    for (bool nonFirst : {false, true})
    {
        for (Fault fault : {Fault::Nonce, Fault::Transport})
        {
            const auto chip = std::make_shared<DUOXChip>();
            TestCommands<DUOXISO7816Commands> commands(chip);
            const auto crypto = std::dynamic_pointer_cast<TestCrypto<DUOXCrypto>>(chip->getCrypto());
            seedSession(*crypto, CM_DUOX);
            crypto->d_currentAid = 0x123456;
            const auto key = makeKey(commands.card.key);
            crypto->setKey(crypto->d_currentAid, 0, 3, key);
            commands.card.nonFirst = nonFirst;
            commands.card.fault = fault;
            if (nonFirst)
            {
                EXPECT_THROW(commands.authenticateEV2NonFirst(3, key), std::exception);
            }
            else
            {
                EXPECT_THROW(commands.authenticateEV2First(3, key), std::exception);
            }
            expectInvalidated(*crypto);
            EXPECT_EQ(crypto->d_currentAid, 0x123456U);
            EXPECT_EQ(crypto->getKey(0, 3), key);
        }
    }
}
