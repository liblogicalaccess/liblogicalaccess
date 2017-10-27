#include "utils.hpp"
#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>
#include <logicalaccess/crypto/des_helper.hpp>
#include <logicalaccess/crypto/sha.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <vector>

using namespace logicalaccess;

/**
 * Return the current factor depending on the current count.
 *
 * The factor is 731 731 731 731 ... 731 ...
 */
static int get_factor(int count)
{
    if (count % 3 == 0)
        return 7;
    if (count % 3 == 1)
        return 3;
    return 1;
}

uint8_t EPassUtils::compute_mrz_checksum(const std::string &in)
{
    uint64_t sum = 0;
    int count    = 0;

    for (const auto &c : in)
    {
        if (isdigit(c))
        {
            sum += (c - '0') * get_factor(count);
        }
        else if (c == '<')
        {
            sum += 0 * get_factor(count); // 0
        }
        else
        {
            sum += (c - 'A' + 10) * get_factor(count);
        }
        count++;
    }
    return static_cast<uint8_t>(sum % 10);
}

ByteVector EPassUtils::seed_from_mrz(const std::string &mrz)
{
    ByteVector data;
    EXCEPTION_ASSERT_WITH_LOG(mrz.size() >= 28, LibLogicalAccessException,
                              "MRZ is too short.");

    auto itr = mrz.begin();
    data.insert(data.end(), itr, itr + 10); // docno
    itr += 10;
    itr += 3; // skip country code

    data.insert(data.end(), itr, itr + 7); // birth date + checksum
    itr += 7;
    itr += 1; // skip don't know what

    data.insert(data.end(), itr, itr + 7); // expiration date + checksum

    auto sha1 = openssl::SHA1Hash(data);
    return ByteVector(sha1.begin(), sha1.begin() + 16);
}

ByteVector EPassUtils::compute_enc_key(const ByteVector &seed)
{
    ByteVector c = {0, 0, 0, 1};
    ByteVector D = seed;
    D.insert(D.end(), c.begin(), c.end());

    auto H = openssl::SHA1Hash(D);
    return adjust_key_parity(ByteVector(H.begin(), H.begin() + 16));
}

ByteVector EPassUtils::compute_mac_key(const ByteVector &seed)
{
    ByteVector c = {0, 0, 0, 2};
    ByteVector D = seed;
    D.insert(D.end(), c.begin(), c.end());

    auto H = openssl::SHA1Hash(D);
    return adjust_key_parity(ByteVector(H.begin(), H.begin() + 16));
}

ByteVector EPassUtils::adjust_key_parity(const ByteVector &key)
{
    ByteVector ret;

    for (const auto &c : key)
    {
        bool even = true;
        for (int i = 0; i < 8; ++i)
        {
            if ((c >> i) & 0x01)
                even = !even;
        }
        if (even)
            ret.push_back(static_cast<uint8_t>(c ^ 0x01));
        else
            ret.push_back(c);
    }
    return ret;
}

/**
 * Check whether the APDU contains an LE byte.
 */
static bool apdu_has_le(const ByteVector &apdu)
{
    assert(apdu.size() >= 4);
    if (apdu.size() == 4)
        return false;
    auto body_size = distance(apdu.begin() + 4, apdu.end());
    return body_size != apdu[4] + 1;
}

/**
 * Check whether the APDU has any data.
 */
static bool apdu_has_data(const ByteVector &apdu)
{
    assert(apdu.size() >= 4);
    if (apdu.size() == 4)
        return false;
    auto body_size = distance(apdu.begin() + 4, apdu.end());
    return body_size >= apdu[4] + 1; // Maybe we also have an LE byte.
}

ByteVector EPassUtils::encrypt_apdu(const ByteVector &apdu, const ByteVector &ks_enc,
                                    const ByteVector &ks_mac, const ByteVector &ssc)
{
    EXCEPTION_ASSERT_WITH_LOG(apdu.size() >= 4, LibLogicalAccessException,
                              "APDU is too short to be valid.");

    auto ssc_incremented        = increment_ssc(ssc);
    ByteVector cmd_header_nopad = {0x0C};
    cmd_header_nopad.insert(cmd_header_nopad.end(), apdu.begin() + 1, apdu.begin() + 4);
    ByteVector cmd_header = pad(cmd_header_nopad);

    ByteVector do_97;
    ByteVector original_data;
    if (apdu_has_le(apdu))
    {
        if (apdu_has_data(apdu))
        {
            original_data = ByteVector(apdu.begin() + 4, apdu.end() - 1);
        }
        do_97 = {0x97, 0x01, *(apdu.end() - 1)};
    }
    else if (apdu_has_data(apdu))
    {
        original_data = ByteVector(apdu.begin() + 5, apdu.end());
    }
    ByteVector encrypted_data = DESHelper::DESEncrypt(pad(original_data), ks_enc, {});

    ByteVector do_87;
    if (apdu_has_data(apdu)) // LC -- do we have any data?
    {
        do_87 = {0x87, static_cast<uint8_t>(encrypted_data.size() + 1), 0x01};
        do_87.insert(do_87.end(), encrypted_data.begin(), encrypted_data.end());
    }

    ByteVector M;
    M.insert(M.end(), cmd_header.begin(), cmd_header.end());
    M.insert(M.end(), do_87.begin(), do_87.end());
    M.insert(M.end(), do_97.begin(), do_97.end());
    M.insert(M.begin(), ssc_incremented.begin(), ssc_incremented.end());

    ByteVector CC    = compute_mac(pad(M), ks_mac);
    ByteVector do_8E = {0x8E, 0x08};
    do_8E.insert(do_8E.end(), CC.begin(), CC.end());

    ByteVector result;
    result.insert(result.end(), cmd_header_nopad.begin(), cmd_header_nopad.end());
    result.push_back(
        static_cast<uint8_t>(do_87.size() + do_97.size() + do_8E.size())); // LC
    result.insert(result.end(), do_87.begin(), do_87.end());
    result.insert(result.end(), do_97.begin(), do_97.end());
    result.insert(result.end(), do_8E.begin(), do_8E.end());
    result.push_back(0);

    return result;
}

ByteVector EPassUtils::pad(const ByteVector &in)
{
    auto out = in;
    out.push_back(1 << 7);

    while (out.size() % 8 != 0)
        out.push_back(0);
    return out;
}

ByteVector EPassUtils::unpad(const ByteVector &in)
{
    auto out = in;

    while (true)
    {
        bool found = (out.back() == 0x80);
        out.pop_back();
        if (found)
            break;
    }
    return out;
}

ByteVector EPassUtils::increment_ssc(const ByteVector &in)
{
    assert(in.size() == 8);
    ByteVector ret;
    ret.reserve(8);

    uint64_t counter;
    for (int i                                           = 0; i < 8; ++i)
        *(reinterpret_cast<uint8_t *>(&counter) + 7 - i) = in[i];
    counter++;

    for (int i = 0; i < 8; ++i)
        ret.push_back((counter >> 8 * (7 - i)) & 0xFF);
    return ret;
}

ByteVector EPassUtils::compute_mac(const ByteVector &in, const ByteVector &k_mac,
                                   const ByteVector &ssc)
{
    EXCEPTION_ASSERT_WITH_LOG(in.size() % 8 == 0, LibLogicalAccessException,
                              "Data for checksum computation is of improper length.");
    ByteVector kmac_a(k_mac.begin(), k_mac.begin() + 8);
    ByteVector kmac_b(k_mac.begin() + 8, k_mac.end());
    ByteVector y;

    if (ssc.size())
    {
        y = DESHelper::DESEncrypt(ssc, kmac_a, {});
    }
    else
        y = ByteVector(8, 0);

    auto itr = in.begin();
    for (uint32_t i = 0; i < in.size() / 8; ++i)
    {
        ByteVector block(itr, itr + 8);
        itr += 8;
        // xor
        for (int j = 0; j < 8; ++j)
            y[j] ^= block[j];

        y = DESHelper::DESEncrypt(y, kmac_a, {});
    }
    y = DESHelper::DESDecrypt(y, kmac_b, {});
    y = DESHelper::DESEncrypt(y, kmac_a, {});
    return y;
}

static bool rapdu_has_data(const ByteVector &rapdu)
{
    EXCEPTION_ASSERT_WITH_LOG(rapdu.size() >= 3, LibLogicalAccessException,
                              "RAPDU is too short.");
    return rapdu[0] == 0x87;
}

ByteVector EPassUtils::decrypt_rapdu(const ByteVector &rapdu, const ByteVector &ks_enc,
                                     const ByteVector &ks_mac, const ByteVector &ssc)
{
    ByteVector do_87;
    ByteVector do_99;
    ByteVector do_8E;

    auto cpy = ByteVector(rapdu.begin(), rapdu.end() - 2);
    auto itr = cpy.begin();
    if (rapdu_has_data(cpy))
    {
        do_87.insert(do_87.end(), itr, itr + 3);
        itr += 3;
        do_87.insert(do_87.end(), itr, itr + cpy[1] - 1);
        itr += cpy[1] - 1;
    }
    EXCEPTION_ASSERT_WITH_LOG(std::distance(itr, cpy.end()) >= 4,
                              LibLogicalAccessException, "RAPDU is too short");
    do_99.insert(do_99.end(), itr, itr + 4);
    itr += 4;
    EXCEPTION_ASSERT_WITH_LOG(std::distance(itr, cpy.end()) >= 10,
                              LibLogicalAccessException, "RAPDU is too short");
    do_8E.insert(do_8E.end(), itr, itr + 10);
    itr += 10;

    ByteVector K;
    auto incremented_ssc = increment_ssc(ssc);
    K.insert(K.end(), incremented_ssc.begin(), incremented_ssc.end());
    K.insert(K.end(), do_87.begin(), do_87.end());
    K.insert(K.end(), do_99.begin(), do_99.end());

    bool rep = false;
    ByteVector CC;
    if (rep)
        CC = compute_mac(pad(K), ks_mac, ssc);
    else
        CC = compute_mac(pad(K), ks_mac);
    EXCEPTION_ASSERT_WITH_LOG(CC == ByteVector(do_8E.begin() + 2, do_8E.end()),
                              LibLogicalAccessException, "Checksum doesn't match");

    ByteVector decrypted_data;
    if (do_87.size())
    {
        decrypted_data =
            DESHelper::DESDecrypt(ByteVector(do_87.begin() + 3, do_87.end()), ks_enc, {});
        decrypted_data = unpad(decrypted_data);
    }
    decrypted_data.insert(decrypted_data.end(), do_99.begin() + 2, do_99.end());
    return decrypted_data;
}

EPassEFCOM EPassUtils::parse_ef_com(const ByteVector &raw)
{
    EPassEFCOM efcom;
    auto itr             = raw.begin();
    auto has_enough_byte = [&](int len) {
        EXCEPTION_ASSERT_WITH_LOG((std::distance(itr, raw.end()) >= len),
                                  LibLogicalAccessException, "Failed to parse EF.COM");
    };
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x60, LibLogicalAccessException,
                              "Failed to parse EF.COM");
    itr += 2; // skip length

    // LDS version
    has_enough_byte(7);
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x5F && *(itr + 1) == 0x01 && *(itr + 2) == 0x04,
                              LibLogicalAccessException, "Cannot parse EF.COM");
    itr += 3;
    for (int i = 0; i < 4; ++i)
        efcom.lds_version_.push_back(*itr++);

    // Unicode Version
    has_enough_byte(9);
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x5F && *(itr + 1) == 0x36 && *(itr + 2) == 0x06,
                              LibLogicalAccessException, "Cannot parse EF.COM");
    itr += 3;
    for (int i = 0; i < 6; ++i)
        efcom.unicode_version_.push_back(*itr++);

    // Tags
    has_enough_byte(2);
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x5C, LibLogicalAccessException,
                              "Cannot parse EF.COM");
    ++itr;
    auto nb_tags = *itr;
    ++itr;
    has_enough_byte(nb_tags);

    for (int i = 0; i < nb_tags; ++i)
        efcom.tags_.push_back(*itr++);

    return efcom;
}

EPassDG2 EPassUtils::parse_dg2(const ByteVector &raw)
{
    EPassDG2 dg2;

    auto itr             = raw.begin();
    auto has_enough_byte = [&](int len, const std::string &why) {
        EXCEPTION_ASSERT_WITH_LOG((std::distance(itr, raw.end()) >= len),
                                  LibLogicalAccessException,
                                  "Failed to parse DG2: " + why);
    };

    // Initial 4 bytes: 0x75 0x82 + 2 len bytes.
    has_enough_byte(4, "initial DG2 data");
    itr += 4;

    // header + len
    has_enough_byte(5, "initial header");
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x7F && *(itr + 1) == 0x61 && *(itr + 2) == 0x82,
                              LibLogicalAccessException, "Cannot parse DG2");
    uint16_t len = *(itr + 3) << 8 | *(itr + 4);
    itr += 5;

    has_enough_byte(len, "total length");
    has_enough_byte(3,
                    "number of entries"); // Number of bio entry. Tag + len (=1) + value
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0x02 && *(itr + 1) == 0x01,
                              LibLogicalAccessException, "Cannot parse DG2");
    uint8_t nb_bio_entry = *(itr + 2);
    itr += 3;

    for (int i = 0; i < nb_bio_entry; ++i)
        dg2.infos_.push_back(parse_dg2_entry(itr, raw.end()));
    assert(itr == raw.end());

    return dg2;
}

EPassDG2::BioInfo EPassUtils::parse_dg2_entry(ByteVector::const_iterator &itr,
                                              const ByteVector::const_iterator &end)
{
    auto has_enough_byte = [&](int len, const std::string &why) {
        EXCEPTION_ASSERT_WITH_LOG((std::distance(itr, end) >= len),
                                  LibLogicalAccessException,
                                  "Failed to parse DG2 entry: " + why);
    };
    EPassDG2::BioInfo bio;

    has_enough_byte(5, "entry tag + length");
    itr += 5;

    has_enough_byte(2, "entry header + len");
    EXCEPTION_ASSERT_WITH_LOG(*itr == 0xA1, LibLogicalAccessException,
                              "Cannot parse DG2 entry 1");
    uint8_t header_length = *(itr + 1);
    itr += 2;

    has_enough_byte(header_length, "entry header");
    auto expected_itr = itr + header_length;
    parse_dg2_entry_header(bio, itr, itr + header_length);
    assert(itr == expected_itr);
    has_enough_byte(5, "tag for biometric data + length");

    EXCEPTION_ASSERT_WITH_LOG((*itr == 0x5F || *itr == 0x7F) && *(itr + 1) == 0x2E &&
                                  *(itr + 2) == 0x82,
                              LibLogicalAccessException, "Cannot parse DG2 entry 2");
    uint16_t data_length = *(itr + 3) << 8 | *(itr + 4);
    itr += 5;
    has_enough_byte(data_length, "biometric data");
    if (bio.format_type_ == ByteVector{0x00, 0x08})
    {
        has_enough_byte(46, "Ignoring additional unknown data.");
        bio.facial_record_header_ = ByteVector(itr, itr + 14);
        itr += 14;

        // From the Facial Record Data we can find how many Facial Feature
        // are present, and derive the number of bytes before the starts of the image
        // file.
        auto facial_data     = ByteVector(itr, itr + 20);
        uint16_t nb_features = facial_data[4] << 8 | facial_data[5];
        has_enough_byte(20 + nb_features * 8 + 12, "advance to image data");
        itr += 20 + nb_features * 8 + 12;

        uint16_t image_offset = static_cast<uint16_t>(14 + 20 + 12 + (8 * nb_features));
        bio.image_data_       = ByteVector(itr, itr + data_length - image_offset);
        itr += data_length - image_offset;
    }
    else
    {
        bio.raw_bio_data_ = ByteVector(itr, itr + data_length);
        itr += data_length;
    }

    return bio;
}

void EPassUtils::parse_dg2_entry_header(EPassDG2::BioInfo &info,
                                        ByteVector::const_iterator &itr,
                                        const ByteVector::const_iterator &end)
{
    auto has_enough_byte = [&](int len, const std::string &why) {
        EXCEPTION_ASSERT_WITH_LOG((std::distance(itr, end) >= len),
                                  LibLogicalAccessException,
                                  "Failed to parse DG2 entry's header: " + why);
    };
    // We read all available TLV in the Header.

    ByteVector header(itr, end);

    if (*itr == 0x80)
    {
        has_enough_byte(3, "OACI Header: length + content");
        info.header_ = ByteVector(itr + 2, itr + 4);
        itr += 4;
    }
    if (*itr == 0x81)
    {
        has_enough_byte(1, "Element type tag + length");
        uint8_t len = *(itr + 1);
        has_enough_byte(len, "Element type value");
        info.element_type_ = ByteVector(itr + 2, itr + 2 + len);
        itr += 2 + len;
    }
    if (*itr == 0x82)
    {
        has_enough_byte(3, "Element subtype: tag + len + value");
        info.element_subtype_ = ByteVector(itr + 2, itr + 3);
        itr += 3;
    }
    if (*itr == 0x83)
    {
        has_enough_byte(9, "Creation time");
        info.created_at_ = ByteVector(itr + 2, itr + 9);
        itr += 9;
    }
    if (*itr == 0x84)
    {
        has_enough_byte(9, "Validity");
        info.valid_ = ByteVector(itr + 2, itr + 10);
        itr += 10;
    }
    if (*itr == 0x86)
    {
        has_enough_byte(3, "Creator");
        info.header_ = ByteVector(itr + 2, itr + 4);
        itr += 4;
    }
    if (*itr == 0x87)
    {
        has_enough_byte(3, "Format owner");
        info.format_owner_ = ByteVector(itr + 2, itr + 4);
        itr += 4;
    }
    if (*itr == 0x88)
    {
        has_enough_byte(3, "Format type");
        info.format_type_ = ByteVector(itr + 2, itr + 4);
        itr += 4;
    }
}

EPassDG1 EPassUtils::parse_dg1(const ByteVector &raw)
{
    auto end = raw.end();
    auto itr = raw.begin();

    auto has_enough_byte = [&](int len, const std::string &why) {
        EXCEPTION_ASSERT_WITH_LOG((std::distance(itr, end) >= len),
                                  LibLogicalAccessException,
                                  "Failed to parse DG1: " + why);
    };

    auto extract_str = [&](int len) {
        has_enough_byte(len, "");
        auto s = std::string(itr, itr + len);
        itr += len;
        return s;
    };
    EPassDG1 dg1;
    // 88 is content, and 5 are initial bytes whose use is unknown.
    has_enough_byte(88 + 5, "Minimum data size");
    itr += 5;

    dg1.type_            = extract_str(2);
    dg1.emitter_         = extract_str(3);
    dg1.owner_name_      = extract_str(39);
    dg1.doc_no_          = extract_str(9);
    dg1.checksum_doc_no_ = extract_str(1);
    dg1.nationality_     = extract_str(3);

    dg1.birthdate_ = parse_dg1_date(ByteVector(itr, itr + 6), 16);
    itr += 6;
    dg1.checksum_birthdate_ = extract_str(1);

    dg1.gender_ = extract_str(1);

    // all expiration dates are assumed to be 20XX.
    dg1.expiration_ = parse_dg1_date(ByteVector(itr, itr + 6), 100);
    itr += 6;
    dg1.checksum_expiration_ = extract_str(1);

    dg1.optional_data_          = extract_str(14);
    dg1.checksum_optional_data_ = extract_str(1);
    dg1.checksum_               = extract_str(1);
    return dg1;
}

std::chrono::system_clock::time_point EPassUtils::parse_dg1_date(const ByteVector &in,
                                                                 int millenium_limit)
{
    assert(in.size() == 6);
    auto itr = in.begin();
    // Birth date extraction code
    auto year  = stoi(std::string(itr, itr + 2));
    auto month = stoi(std::string(itr + 2, itr + 4));
    auto day   = stoi(std::string(itr + 4, itr + 6));

    tm date;
    memset(&date, 0x00, sizeof(tm));
    date.tm_mon  = month - 1; // tm_mon starts at 0
    date.tm_mday = day;

    // Year is stored on 2 digits. We say that year > millenium_limit means 19XX.
    // Otherwise it means 20XX.
    // Note that tm_year is number of year since 1900.
    if (year > millenium_limit)
        date.tm_year = year;
    else
        date.tm_year = 100 + year;

    return std::chrono::system_clock::from_time_t(mktime(&date));
}
