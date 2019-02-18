#include <logicalaccess/plugins/cards/epass/epasscommands.hpp>
#include <logicalaccess/plugins/cards/epass/epassreadercardadapter.hpp>
#include <cassert>
#include <iomanip>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/crypto/sha.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>

using namespace logicalaccess;

EPassCommands::EPassCommands()
    : EPassCommands(CMD_EPASS)
{
}

EPassCommands::EPassCommands(std::string ct)
    : Commands(ct)
{
    crypto_ = std::make_shared<EPassCrypto>();
}

bool EPassCommands::authenticate(const std::string &mrz)
{
    LLA_LOG_CTX("EPassIdentityCardService::authenticate");

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
            assert(crypto_);
            crypto_->reset(mrz);

            std::shared_ptr<ISO7816ReaderCardAdapter> rcu =
                std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
                    getReaderCardAdapter());
            assert(rcu);
            auto iso7816cmd = getISO7816Commands();
            auto challenge  = iso7816cmd->getChallenge(8);
            auto rpcd       = crypto_->step1(challenge);
            auto rpicc1 = iso7816cmd->externalAuthenticate(0x00, true, 0x00, rpcd, 0x28);
            // drop status bytes.
            EXCEPTION_ASSERT_WITH_LOG(rpicc1.size() == 40, LibLogicalAccessException,
                                      "Unexpected response length");
            return crypto_->step2(rpicc1);
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
    getISO7816Commands()->selectFile(P1_SELECT_EF_UNDER_CURRENT_DF, P2_RETURN_NO_FCI,
                                     {0x01, 0x1E});

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

    getISO7816Commands()->selectFile(app_id);
    current_app_ = app_id;
    return true;
}

bool EPassCommands::selectIssuerApplication()
{
    LLA_LOG_CTX("EPassIdentityCardService::selectIssuerApplication");
    return selectApplication({0xA0, 0, 0, 0x02, 0x47, 0x10, 0x01});
}

void EPassCommands::setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter)
{
    Commands::setReaderCardAdapter(adapter);
    auto epass_rca = std::dynamic_pointer_cast<EPassReaderCardAdapter>(adapter);
    if (epass_rca)
        epass_rca->setEPassCrypto(crypto_);
}

EPassDG1 EPassCommands::readDG1() const
{
    getISO7816Commands()->selectFile(P1_SELECT_EF_UNDER_CURRENT_DF, P2_RETURN_NO_FCI,
                                     {0x01, 0x01});
    auto raw = readEF(1, 1);
    return EPassUtils::parse_dg1(raw);
}

EPassDG2 EPassCommands::readDG2()
{
    LLA_LOG_CTX("EPassCommands::readDG2");
    getISO7816Commands()->selectFile(P1_SELECT_EF_UNDER_CURRENT_DF, P2_RETURN_NO_FCI,
                                     {0x01, 0x02});

    // File tag is 2 bytes and size is 2 bytes too.
    auto dg2_raw = readEF(2, 2);

    auto dg2 = EPassUtils::parse_dg2(dg2_raw);
    return dg2;
}

ByteVector EPassCommands::readEF(uint8_t size_bytes, uint8_t size_offset) const
{
    ByteVector ef_raw;
    uint8_t initial_read_len = static_cast<uint8_t>(size_bytes + size_offset);

    auto iso7816cmd = getISO7816Commands();
    auto data       = iso7816cmd->readBinary(initial_read_len, 0);
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
        data            = iso7816cmd->readBinary(to_read, offset);
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

    getISO7816Commands()->selectFile(P1_SELECT_EF_UNDER_CURRENT_DF, P2_RETURN_NO_FCI,
                                     {0x01, 0x1D});
    auto tmp = readEF(2, 2);
}

ByteVector EPassCommands::compute_hash(const ByteVector &file_id) const
{
    getISO7816Commands()->selectFile(P1_SELECT_EF_UNDER_CURRENT_DF, P2_RETURN_NO_FCI,
                                     file_id);

    ByteVector content;
    if (file_id == ByteVector{1, 1})
        content = readEF(1, 1);
    else if (file_id == ByteVector{1, 2})
        content = readEF(2, 2);

    // Hash algorithm can vary.
    return openssl::SHA1Hash(content);
    // return openssl::SHA256Hash(content);
}
