#pragma once

#include <chrono>
#include <logicalaccess/plugins/cards/epass/lla_cards_epass_api.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{

/**
 * A data structure representing the content of the
 * EF.COM file.
 */
struct LLA_CARDS_EPASS_API EPassEFCOM
{
    std::string lds_version_;
    std::string unicode_version_;

    ByteVector tags_;
};

struct LLA_CARDS_EPASS_API EPassDG2
{
    struct BioInfo
    {
        ByteVector header_;
        ByteVector element_type_;
        ByteVector element_subtype_;
        ByteVector created_at_;
        ByteVector valid_;
        ByteVector creator_;

        /**
         * Below are mandatory
         */

        ByteVector format_owner_;
        ByteVector format_type_;

        // 14 bytes
        ByteVector facial_record_header_;

        // Data except the image content
        ByteVector facial_record_data_;

        // Write that to file, get an image
        ByteVector image_data_;

        // For unknown format we simply binary copy the bio data.
        ByteVector raw_bio_data_;
    };

    std::vector<BioInfo> infos_;
};

struct LLA_CARDS_EPASS_API EPassDG1
{
    std::string type_;
    std::string emitter_;
    std::string owner_name_;
    std::string doc_no_;
    std::string checksum_doc_no_;
    std::string nationality_;
    std::chrono::system_clock::time_point birthdate_;
    std::string checksum_birthdate_;
    std::string gender_;
    std::chrono::system_clock::time_point expiration_;
    std::string checksum_expiration_;
    std::string optional_data_;
    std::string checksum_optional_data_;
    std::string checksum_;
};


/**
 * An helper class that provide various utilities regarding e-passport.
 *
 * MRZ: Machine Readable Zone.
 * Utility to compute checksum, extract information from ROC-B string, etc.
 */
class LLA_CARDS_EPASS_API EPassUtils
{
  public:
    /**
     * Compute a MRZ.
     *
     * See documentation part 1 volume 1 $16.
     */
    static uint8_t compute_mrz_checksum(const std::string &in);

    /**
     * Extract Kseed from MRZ.
     */
    static ByteVector seed_from_mrz(const std::string &mrz);

    /**
     * Parse the EF.COM content and build an EPassEFCOM object with
     * the proper content.
     */
    static EPassEFCOM parse_ef_com(const ByteVector &raw);

    /**
     * Parse the DG2 file content
     */
    static EPassDG2 parse_dg2(const ByteVector &raw);

    static EPassDG2::BioInfo parse_dg2_entry(ByteVector::const_iterator &itr,
                                             const ByteVector::const_iterator &end);

    static void parse_dg2_entry_header(EPassDG2::BioInfo &info,
                                       ByteVector::const_iterator &itr,
                                       const ByteVector::const_iterator &end);

    /**
     * Parse the Data Group 1.
     */
    static EPassDG1 parse_dg1(const ByteVector &raw);

    /**
     * Create a timepoint object from an YYMMDD ascii array.
     *
     * @param millenium_limit represents highest value for a date
     * to be considered 19XX instead of 20XX;
     */
    static std::chrono::system_clock::time_point parse_dg1_date(const ByteVector &in,
                                                                int millenium_limit);
};
}
