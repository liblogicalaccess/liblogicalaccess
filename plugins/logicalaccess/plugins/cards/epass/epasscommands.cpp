#include <logicalaccess/plugins/cards/epass/epasscommands.hpp>
#include <logicalaccess/plugins/cards/epass/epassreadercardadapter.hpp>
#include <cassert>
#include <iomanip>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/crypto/sha.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>

using namespace logicalaccess;

EPassCommands::EPassCommands()
    : Commands(CMD_EPASS)
{
}

EPassCommands::EPassCommands(std::string ct)
    : Commands(ct)
{
}

static ByteVector get_challenge(std::shared_ptr<ISO7816ReaderCardAdapter> rca)
{
    ByteVector result;

    result = rca->sendAPDUCommand(0, 0x84, 0x00, 0x00, 8);
    EXCEPTION_ASSERT_WITH_LOG(result.size() >= 2, LibLogicalAccessException,
                              "GetChallenge response is too short.");
    if (result[result.size() - 2] == 0x90 && result[result.size() - 1] == 0x00)
    {
        return ByteVector(result.begin(), result.end() - 2);
    }
    return result;
}

bool EPassCommands::authenticate(const std::string &mrz)
{
    LLA_LOG_CTX("EPassIdentityService::authenticate");
    // We perform a classic ISO7816 authenticate. However, there are some caveats.
    //     + Some epassport chip returns an error when we try to authenticate while
    //       we are already authenticated.
    //     + This situation can happens because the IdentityService
    //       do not assume any authentication status, and perform authentication anytime
    //       a data is required (if cache misses).
    //
    // The workaround consist of authenticating multiple time, until it works.
    const int TRY_COUNT = 2;

    for (int i = 0; i < TRY_COUNT; ++i)
    {
        try
        {
            crypto_ = std::make_shared<EPassCrypto>(mrz);
            cryptoChanged();

            std::shared_ptr<ISO7816ReaderCardAdapter> rcu =
                std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
                    getReaderCardAdapter());
            assert(rcu);
            auto challenge = get_challenge(rcu);
            auto tmp       = crypto_->step1(challenge);

            tmp = rcu->sendAPDUCommand(0x00, 0x82, 0x00, 0x00, 0x28, tmp, 0x28);
            // drop status bytes.
            EXCEPTION_ASSERT_WITH_LOG(tmp.size() == 40, LibLogicalAccessException,
                                      "Unexpected response length");
            return crypto_->step2(tmp);
        }
        catch (const CardException &e)
        {
            if ((i == TRY_COUNT - 1) ||
                e.error_code() != CardException::ErrorType::SECURITY_STATUS)
                throw;
        }
    }
    assert(0);
    throw std::runtime_error("The impossible happened.");
}

EPassEFCOM EPassCommands::readEFCOM() const
{
    ByteVector ef_com_raw;
    selectEF({0x01, 0x1E});

    auto data = readEF(1, 1);
    EXCEPTION_ASSERT_WITH_LOG(data.size() >= 10, LibLogicalAccessException,
                              "EF.COM data seems too short");
    EXCEPTION_ASSERT_WITH_LOG(data[0] == 0x60, LibLogicalAccessException,
                              "Invalid Tag for EF.COM file.");
    return EPassUtils::parse_ef_com(data);
}

bool EPassCommands::selectApplication(const ByteVector &app_id)
{
    if (app_id == current_app_)
    {
        LOG(INFOS) << "Not selecting application " << app_id
                   << " because it is already selected";
        return true;
    }

    std::shared_ptr<ISO7816ReaderCardAdapter> rca =
        std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter());
    assert(rca);
    auto ret = rca->sendAPDUCommand(0x00, 0xA4, 0x04, 0x0C, (int)(app_id.size()), app_id);
    current_app_ = app_id;
    return true;
}

bool EPassCommands::selectEF(const ByteVector &file_id) const
{
    std::shared_ptr<ISO7816ReaderCardAdapter> rca =
        std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter());
    assert(rca);
    auto ret =
        rca->sendAPDUCommand(0x00, 0xA4, 0x02, 0x0C, (int)(file_id.size()), file_id);
    return true;
}

bool EPassCommands::selectIssuerApplication()
{
    LLA_LOG_CTX("EPassIdentityService::selectIssuerApplication");
    return selectApplication({0xA0, 0, 0, 0x02, 0x47, 0x10, 0x01});
}

void EPassCommands::setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter)
{
    Commands::setReaderCardAdapter(adapter);
    auto epass_rca = std::dynamic_pointer_cast<EPassReaderCardAdapter>(adapter);
    if (epass_rca)
        epass_rca->setEPassCrypto(crypto_);
}

void EPassCommands::cryptoChanged() const
{
    auto epass_rca =
        std::dynamic_pointer_cast<EPassReaderCardAdapter>(getReaderCardAdapter());
    if (epass_rca)
        epass_rca->setEPassCrypto(crypto_);
}

ByteVector EPassCommands::readBinary(uint16_t offset, uint8_t length) const
{
    uint8_t p1 = 0;
    uint8_t p2 = 0;
    p1         = static_cast<uint8_t>(0x7f & (offset >> 8));
    p2         = static_cast<uint8_t>(offset & 0xFF);

    std::shared_ptr<ISO7816ReaderCardAdapter> rca =
        std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(getReaderCardAdapter());
    if (length)
        return rca->sendAPDUCommand(0x00, 0xB0, p1, p2, length);
    return rca->sendAPDUCommand(0x00, 0xB0, p1, p2);
}

EPassDG1 EPassCommands::readDG1() const
{
    selectEF({0x01, 0x01});
    auto raw = readEF(1, 1);
    return EPassUtils::parse_dg1(raw);
}

EPassDG2 EPassCommands::readDG2()
{
    LLA_LOG_CTX("EPassCommands::readDG2");
    selectEF({0x01, 0x02});

    // File tag is 2 bytes and size is 2 bytes too.
    auto dg2_raw = readEF(2, 2);

    auto dg2 = EPassUtils::parse_dg2(dg2_raw);
    return dg2;
}

ByteVector EPassCommands::readEF(uint8_t size_bytes, uint8_t size_offset) const
{
    ByteVector ef_raw;
    uint8_t initial_read_len = static_cast<uint8_t>(size_bytes + size_offset);

    auto data = readBinary(0, initial_read_len);
    EXCEPTION_ASSERT_WITH_LOG(data.size() == initial_read_len, LibLogicalAccessException,
                              "Wrong data size.");
    ef_raw.insert(ef_raw.end(), data.begin(), data.end());

    // compute the length of the file, based on the number of bytes representing the
    // size
    // and the initial offset of those bytes.
    uint16_t length = 0;
    for (int i = 0; i < size_bytes; ++i)
        length |= data[size_offset + i] << (size_bytes - i - 1) * 8;

    uint16_t offset = initial_read_len;
    while (length)
    {
        // somehow reading more will cause invalid checksum.
        // todo: investigate.
        uint8_t to_read = static_cast<uint8_t>(length > 100 ? 100 : length);
        data            = readBinary(offset, to_read);
        EXCEPTION_ASSERT_WITH_LOG(data.size() == to_read, LibLogicalAccessException,
                                  "Wrong data size");
        ef_raw.insert(ef_raw.end(), data.begin(), data.end());
        offset += (uint8_t)(data.size());
        length -= (uint8_t)(data.size());
    }
    return ef_raw;
}

void EPassCommands::readSOD() const
{
    // SOD is ASN.1
    // For now we are not able to use it.
    auto hash_1 = compute_hash({1, 1});
    auto hash_2 = compute_hash({1, 2});

    selectEF({0x01, 0x1D});
    auto tmp = readEF(2, 2);
}

ByteVector EPassCommands::compute_hash(const ByteVector &file_id) const
{
    selectEF(file_id);

    ByteVector content;
    if (file_id == ByteVector{1, 1})
        content = readEF(1, 1);
    else if (file_id == ByteVector{1, 2})
        content = readEF(2, 2);

    // Hash algorithm can vary.
    return openssl::SHA1Hash(content);
    // return openssl::SHA256Hash(content);
}
