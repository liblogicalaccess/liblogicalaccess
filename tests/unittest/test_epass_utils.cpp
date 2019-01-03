#include "logicalaccess/lla_fwd.hpp"
#include "logicalaccess/plugins/cards/epass/utils.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/epass/epasscrypto.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>

using namespace logicalaccess;

TEST(test_epass_utils, test_zla_checksum)
{
    ASSERT_EQ(3, EPassUtils::compute_mrz_checksum("520727"));
    ASSERT_EQ(7, EPassUtils::compute_mrz_checksum("790410"));
    ASSERT_EQ(7, EPassUtils::compute_mrz_checksum("200618"));
}

TEST(test_epass_utils, test_mrz_seed)
{
    auto mrz_line_2 = "L898902C<3UTO6908061F9406236ZE184226B<<<<<14";
    auto seed       = BufferHelper::fromHexString("239AB9CB282DAF66231DC5A4DF6BFBAE");

    ASSERT_EQ(seed, EPassUtils::seed_from_mrz(mrz_line_2));
}

TEST(test_epass_utils, test_adjust_parity)
{
    auto key1 = BufferHelper::fromHexString("AB94FCEDF2664EDF");
    auto key2 = BufferHelper::fromHexString("B9B291F85D7F77F2");

    auto valid_key1 = BufferHelper::fromHexString("AB94FDECF2674FDF");
    auto valid_key2 = BufferHelper::fromHexString("B9B391F85D7F76F2");

    ASSERT_EQ(valid_key1, EPassCrypto::adjust_key_parity(key1));
    ASSERT_EQ(valid_key2, EPassCrypto::adjust_key_parity(key2));
}

TEST(test_epass_utils, test_compute_base_key)
{
    auto Kenc = BufferHelper::fromHexString("AB94FDECF2674FDFB9B391F85D7F76F2");
    auto Kmac = BufferHelper::fromHexString("7962D9ECE03D1ACD4C76089DCE131543");
    auto seed = BufferHelper::fromHexString("239AB9CB282DAF66231DC5A4DF6BFBAE");

    ASSERT_EQ(Kenc, EPassCrypto().compute_enc_key(seed));
    ASSERT_EQ(Kmac, EPassCrypto().compute_mac_key(seed));
}

TEST(test_epass_utils, test_authenticate_1)
{
    EPassCrypto c("L898902C<3UTO6908061F9406236ZE184226B<<<<<14");
    auto mutual_auth_data =
        c.step1(BufferHelper::fromHexString("4608F91988702212"),
                BufferHelper::fromHexString("781723860C06C226"),
                BufferHelper::fromHexString("0B795240CB7049B01C19B33E32804F0B"));

    ASSERT_EQ(BufferHelper::fromHexString("72C29C2371CC9BDB65B779B8E8D37B29ECC154AA5"
                                          "6A8799FAE2F498F76ED92F25F1448EEA8AD90A7"),
              mutual_auth_data);
}

TEST(test_epass_utils, test_authenticate_2)
{
    EPassCrypto c("L898902C<3UTO6908061F9406236ZE184226B<<<<<14");
    auto mutual_auth_data =
        c.step1(BufferHelper::fromHexString("4608F91988702212"),
                BufferHelper::fromHexString("781723860C06C226"),
                BufferHelper::fromHexString("0B795240CB7049B01C19B33E32804F0B"));

    ASSERT_EQ(BufferHelper::fromHexString("72C29C2371CC9BDB65B779B8E8D37B29ECC154AA5"
                                          "6A8799FAE2F498F76ED92F25F1448EEA8AD90A7"),
              mutual_auth_data);

    auto auth_response =
        BufferHelper::fromHexString("46B9342A41396CD7386BF5803104D7CEDC122B9132139BA"
                                    "F2EEDC94EE178534F2F2D235D074D7449");
    ASSERT_TRUE(c.step2(auth_response));
    // We can now retrieve key from the crypto object.
    ASSERT_EQ(BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
              c.get_session_enc_key());
    ASSERT_EQ(BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
              c.get_session_mac_key());
    ASSERT_EQ(BufferHelper::fromHexString("887022120C06C226"),
              c.get_send_session_counter());
}

TEST(test_epass_utils, test_secure_messaging)
{
    auto clear_apdu     = BufferHelper::fromHexString("00A4020C02011E");
    auto encrypted_apdu = EPassCrypto().encrypt_apdu(std::make_shared<openssl::DESCipher>(),
        clear_apdu, BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C226"));

    ASSERT_EQ(BufferHelper::fromHexString(
                  "0CA4020C158709016375432908C044F68E08BF8B92D635FF24F800"),
              encrypted_apdu);

    // Decrypt response
    auto decrypted_response = EPassCrypto().decrypt_rapdu(
            std::make_shared<openssl::DESCipher>(),
        BufferHelper::fromHexString("990290008E08FA855A5D4C50A8ED9000"),
        BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C227"));
    ASSERT_EQ(BufferHelper::fromHexString("9000"), decrypted_response);
}

TEST(test_epass_utils, test_secure_messaging_2)
{
    auto clear_apdu     = BufferHelper::fromHexString("00B0000004");
    auto encrypted_apdu = EPassCrypto().encrypt_apdu(std::make_shared<openssl::DESCipher>(),
        clear_apdu, BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C228"));

    ASSERT_EQ(BufferHelper::fromHexString("0CB000000D9701048E08ED6705417E96BA5500"),
              encrypted_apdu);

    // Decrypt response
    auto decrypted_response = EPassCrypto().decrypt_rapdu(std::make_shared<openssl::DESCipher>(),
        BufferHelper::fromHexString(
            "8709019FF0EC34F9922651990290008E08AD55CC17140B2DED9000"),
        BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C229"));
    ASSERT_EQ(BufferHelper::fromHexString("60145F019000"), decrypted_response);
}

TEST(test_epass_utils, test_secure_messaging_3)
{
    auto clear_apdu     = BufferHelper::fromHexString("00B0000412");
    auto encrypted_apdu = EPassCrypto().encrypt_apdu(std::make_shared<openssl::DESCipher>(),
        clear_apdu, BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C22A"));

    ASSERT_EQ(BufferHelper::fromHexString("0CB000040D9701128E082EA28A70F3C7B53500"),
              encrypted_apdu);

    // Decrypt response
    auto decrypted_response = EPassCrypto().decrypt_rapdu(std::make_shared<openssl::DESCipher>(),
        BufferHelper::fromHexString("871901FB9235F4E4037F2327DCC8964F1F9B8C30F42C8E2"
                                    "FFF224A990290008E08C8B2787EAEA07D749000"),
        BufferHelper::fromHexString("979EC13B1CBFE9DCD01AB0FED307EAE5"),
        BufferHelper::fromHexString("F1CB1F1FB5ADF208806B89DC579DC1F8"),
        BufferHelper::fromHexString("887022120C06C22B"));
    ASSERT_EQ(BufferHelper::fromHexString("04303130365F36063034303030305C0261759000"),
              decrypted_response);
}

TEST(test_epass_utils, test_parse_ef_com)
{
    auto raw =
        BufferHelper::fromHexString("60165F0104303130375F36063034303030305C046175636E");
    auto ef_com = EPassUtils::parse_ef_com(raw);

    ASSERT_EQ("0107", ef_com.lds_version_);
    ASSERT_EQ("040000", ef_com.unicode_version_);
    auto tags = std::vector<uint8_t>({0x61, 0x75, 0x63, 0x6E});
    ASSERT_EQ(tags, ef_com.tags_);
}

TEST(test_epass_utils, test_parse_dg2_entry_header)
{
    ByteVector header = {0x80, 0x2,  0x1, 0x1, 0x81, 0x1,  0x2, 0x82, 0x1,
                         0x0,  0x87, 0x2, 0x1, 0x1,  0x88, 0x2, 0x0,  0x8};
    EPassDG2::BioInfo info;

    auto b = header.cbegin();
    auto e = header.cend();

    EPassUtils::parse_dg2_entry_header(info, b, e);
    ASSERT_EQ((ByteVector{0x1, 0x1}), info.header_);
    ASSERT_EQ((ByteVector{0x2}), info.element_type_);
    ASSERT_EQ((ByteVector{0x0}), info.element_subtype_);
    ASSERT_EQ((ByteVector{0x1, 0x1}), info.format_owner_);
    ASSERT_EQ((ByteVector{0x0, 0x8}), info.format_type_);
}

TEST(test_epass_utils, test_parse_dg1)
{
    auto binary =
        ByteVector{0x61, 0x5b, 0x5f, 0x1f, 0x58, 0x50, 0x3c, 0x55, 0x54, 0x4f, 0x41, 0x4e,
                   0x44, 0x45, 0x52, 0x53, 0x4f, 0x4e, 0x3c, 0x3c, 0x4a, 0x41, 0x4e, 0x45,
                   0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
                   0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
                   0x3c, 0x57, 0x37, 0x47, 0x43, 0x48, 0x39, 0x5a, 0x59, 0x32, 0x34, 0x55,
                   0x54, 0x4f, 0x37, 0x39, 0x30, 0x34, 0x31, 0x30, 0x37, 0x46, 0x32, 0x30,
                   0x30, 0x36, 0x31, 0x38, 0x37, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
                   0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x32};

    EPassDG1 dg1 = EPassUtils::parse_dg1(binary);
    ASSERT_EQ("P<", dg1.type_);
    ASSERT_EQ("UTO", dg1.emitter_);
    ASSERT_EQ("ANDERSON<<JANE<<<<<<<<<<<<<<<<<<<<<<<<<", dg1.owner_name_);
    ASSERT_EQ("W7GCH9ZY2", dg1.doc_no_);
    ASSERT_EQ("4", dg1.checksum_doc_no_);
    ASSERT_EQ("UTO", dg1.nationality_);

	#ifdef __linux__
    std::tm date          = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	#else
    std::tm date = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	#endif
    date.tm_mon           = 3;
    date.tm_mday          = 10;
    date.tm_year          = 79;
    std::time_t time_temp = std::mktime(&date);
    auto birthdate        = std::chrono::system_clock::from_time_t(time_temp);
    ASSERT_EQ(birthdate, dg1.birthdate_);
    ASSERT_EQ("7", dg1.checksum_birthdate_);


	#ifdef __linux__
    std::tm date2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	#else
	std::tm date2 = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	#endif
    date2.tm_mon           = 5;
    date2.tm_mday          = 18;
    date2.tm_year          = 120;
    std::time_t time_temp2 = std::mktime(&date2);
    auto expiration_date   = std::chrono::system_clock::from_time_t(time_temp2);
    ASSERT_EQ(expiration_date, dg1.expiration_);
    ASSERT_EQ("7", dg1.checksum_expiration_);

    ASSERT_EQ("F", dg1.gender_);
    ASSERT_EQ("<<<<<<<<<<<<<<", dg1.optional_data_);
    ASSERT_EQ("<", dg1.checksum_optional_data_);
    ASSERT_EQ("2", dg1.checksum_);
}