#include <reader.h>
#include <logicalaccess/utils.hpp>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "pcscfeatures.hpp"

using namespace logicalaccess;

/**
 * Various defines from PCSC Lite, not available on Windows.
 */
#ifndef FEATURE_VERIFY_PIN_START
#define FEATURE_VERIFY_PIN_START 0x01
#endif
#ifndef FEATURE_VERIFY_PIN_FINISH
#define FEATURE_VERIFY_PIN_FINISH 0x02
#endif
#ifndef FEATURE_MODIFY_PIN_START
#define FEATURE_MODIFY_PIN_START 0x03
#endif
#ifndef FEATURE_MODIFY_PIN_FINISH
#define FEATURE_MODIFY_PIN_FINISH 0x04
#endif
#ifndef FEATURE_GET_KEY_PRESSED
#define FEATURE_GET_KEY_PRESSED 0x05
#endif
#ifndef FEATURE_VERIFY_PIN_DIRECT
#define FEATURE_VERIFY_PIN_DIRECT 0x06
#endif
#ifndef FEATURE_MODIFY_PIN_DIRECT
#define FEATURE_MODIFY_PIN_DIRECT 0x07
#endif
#ifndef FEATURE_MCT_READER_DIRECT
#define FEATURE_MCT_READER_DIRECT 0x08
#endif
#ifndef FEATURE_MCT_UNIVERSAL
#define FEATURE_MCT_UNIVERSAL 0x09
#endif
#ifndef FEATURE_IFD_PIN_PROPERTIES
#define FEATURE_IFD_PIN_PROPERTIES 0x0A
#endif
#ifndef FEATURE_ABORT
#define FEATURE_ABORT 0x0B
#endif
#ifndef FEATURE_SET_SPE_MESSAGE
#define FEATURE_SET_SPE_MESSAGE 0x0C
#endif
#ifndef FEATURE_VERIFY_PIN_DIRECT_APP_ID
#define FEATURE_VERIFY_PIN_DIRECT_APP_ID 0x0D
#endif
#ifndef FEATURE_MODIFY_PIN_DIRECT_APP_ID
#define FEATURE_MODIFY_PIN_DIRECT_APP_ID 0x0E
#endif
#ifndef FEATURE_WRITE_DISPLAY
#define FEATURE_WRITE_DISPLAY 0x0F
#endif
#ifndef FEATURE_GET_KEY
#define FEATURE_GET_KEY 0x10
#endif
#ifndef FEATURE_IFD_DISPLAY_PROPERTIES
#define FEATURE_IFD_DISPLAY_PROPERTIES 0x11
#endif
#ifndef FEATURE_GET_TLV_PROPERTIES
#define FEATURE_GET_TLV_PROPERTIES 0x12
#endif
#ifndef FEATURE_CCID_ESC_COMMAND
#define FEATURE_CCID_ESC_COMMAND 0x13
#endif
#ifndef FEATURE_EXECUTE_PACE
#define FEATURE_EXECUTE_PACE 0x14
#endif

/**
 * Defines of TLV properties opcode, as provided by PCSC Lite
 */
#ifndef PCSCv2_PART10_PROPERTY_wLcdLayout
#define PCSCv2_PART10_PROPERTY_wLcdLayout 1
#endif
#ifndef PCSCv2_PART10_PROPERTY_bEntryValidationCondition
#define PCSCv2_PART10_PROPERTY_bEntryValidationCondition 2
#endif
#ifndef PCSCv2_PART10_PROPERTY_bTimeOut2
#define PCSCv2_PART10_PROPERTY_bTimeOut2 3
#endif
#ifndef PCSCv2_PART10_PROPERTY_wLcdMaxCharacters
#define PCSCv2_PART10_PROPERTY_wLcdMaxCharacters 4
#endif
#ifndef PCSCv2_PART10_PROPERTY_wLcdMaxLines
#define PCSCv2_PART10_PROPERTY_wLcdMaxLines 5
#endif
#ifndef PCSCv2_PART10_PROPERTY_bMinPINSize
#define PCSCv2_PART10_PROPERTY_bMinPINSize 6
#endif
#ifndef PCSCv2_PART10_PROPERTY_bMaxPINSize
#define PCSCv2_PART10_PROPERTY_bMaxPINSize 7
#endif
#ifndef PCSCv2_PART10_PROPERTY_sFirmwareID
#define PCSCv2_PART10_PROPERTY_sFirmwareID 8
#endif
#ifndef PCSCv2_PART10_PROPERTY_bPPDUSupport
#define PCSCv2_PART10_PROPERTY_bPPDUSupport 9
#endif
#ifndef PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize
#define PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize 10
#endif
#ifndef PCSCv2_PART10_PROPERTY_wIdVendor
#define PCSCv2_PART10_PROPERTY_wIdVendor 11
#endif
#ifndef PCSCv2_PART10_PROPERTY_wIdProduct
#define PCSCv2_PART10_PROPERTY_wIdProduct 12
#endif

PCSCFeatures::PCSCFeatures(PCSCConnection *connection)
    : connection_(connection)
{
}

PCSCFeatures::Feature::Feature(uint8_t tag, uint32_t ioctl, const std::string &name)
    : tag_(tag)
    , ioctl_(ioctl)
    , name_(name)
{
}

PCSCFeatures::Property::Property(uint8_t tag, uint32_t data, const std::string &name)
    : tag_(tag)
    , value_(data)
    , name_(name)
{
}

std::vector<PCSCFeatures::Feature> PCSCFeatures::getFeatures()
{
    auto bytes = fetch_features_bytes();
    std::vector<Feature> features;

    for (auto start = bytes.begin(); start != bytes.end();)
    {
        features.push_back(extract_feature_from_bytes(start, bytes.end()));
    }
    return features;
}

ByteVector PCSCFeatures::fetch_features_bytes()
{
    std::vector<uint8_t> out(250);
    uint64_t out_length = 0;
    auto rv = SCardControl(connection_->getHandle(), CM_IOCTL_GET_FEATURE_REQUEST,
                           NULL, 0, &out[0], out.size(), &out_length);
    if (rv != SCARD_S_SUCCESS)
    {
        std::string error_msg = PCSCConnection::strerror(rv);
        THROW_EXCEPTION_WITH_LOG(CardException, error_msg);
    }
    EXCEPTION_ASSERT_WITH_LOG(out.size() % sizeof(PCSC_TLV_STRUCTURE) != 0,
                              LibLogicalAccessException,
                              "TLV values size are unexpected.");
    out.resize(out_length);
    return out;
}

PCSCFeatures::Feature
PCSCFeatures::extract_feature_from_bytes(ByteVector::iterator &it,
                                         const ByteVector::iterator &end)
{
    auto check_enough_bytes = [&](int n)
    {
        EXCEPTION_ASSERT_WITH_LOG(
            std::distance(it, end) >= n, LibLogicalAccessException,
            "Parsing GET_FEATURE_REQUESTS response failed. Not enough bytes.");
    };
    check_enough_bytes(2);
    uint8_t tag    = *it++;
    uint8_t length = *it++;
    check_enough_bytes(length);
    // Length shall be 4.
    EXCEPTION_ASSERT_WITH_LOG(length == 4, LibLogicalAccessException,
                              "Length of the TLV"
                              " feature description is incorrect: " +
                                  std::to_string(length));

    uint32_t data = 0;
    for (int i = 0; i < 4; ++i)
        data |= *it++ << (i * 8);
    data = lla_ntohl(data);
    switch (tag)
    {
    case FEATURE_VERIFY_PIN_DIRECT:
        return Feature(tag, data, "FEATURE_VERIFY_PIN_DIRECT");
    case FEATURE_MODIFY_PIN_DIRECT:
        return Feature(tag, data, "FEATURE_MODIFY_PIN_DIRECT");
    case FEATURE_IFD_PIN_PROPERTIES:
        return Feature(tag, data, "FEATURE_IFD_PIN_PROPERTIES");
    case FEATURE_MCT_READER_DIRECT:
        return Feature(tag, data, "FEATURE_MCT_READER_DIRECT");
    case FEATURE_GET_TLV_PROPERTIES:
        return Feature(tag, data, "FEATURE_GET_TLV_PROPERTIES");
    case FEATURE_CCID_ESC_COMMAND:
        return Feature(tag, data, "FEATURE_CCID_ESC_COMMAND");
    }
    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "Tag " + std::to_string(+tag) + " is not recognized");
}

ByteVector
PCSCFeatures::fetch_tlv_properties_bytes(const Feature &tvl_properties_features)
{
    std::vector<uint8_t> out(250);
    uint64_t out_length = 0;

    auto rv = SCardControl(connection_->getHandle(), tvl_properties_features.ioctl_,
                           NULL, 0, &out[0], out.size(), &out_length);
    if (rv != SCARD_S_SUCCESS)
    {
        std::string error_msg = PCSCConnection::strerror(rv);
        THROW_EXCEPTION_WITH_LOG(CardException, error_msg);
    }

    out.resize(out_length);
    return out;
}

std::vector<PCSCFeatures::Property> PCSCFeatures::getTLVProperties()
{
    ByteVector bytes;
    for (const auto &f : getFeatures())
    {
        // Find the feature to get the IOCTL for fetching
        // properties
        if (f.tag_ == FEATURE_GET_TLV_PROPERTIES)
        {
            bytes = fetch_tlv_properties_bytes(f);
            break;
        }
    }

    std::vector<Property> properties;
    for (auto start = bytes.begin(); start != bytes.end();)
    {
        properties.push_back(extract_property_from_bytes(start, bytes.end()));
    }
    return properties;
}

PCSCFeatures::Property
PCSCFeatures::extract_property_from_bytes(ByteVector::iterator &it,
                                          const ByteVector::iterator &end)
{
    auto check_enough_bytes = [&](int n)
    {
        EXCEPTION_ASSERT_WITH_LOG(
            std::distance(it, end) >= n, LibLogicalAccessException,
            "Parsing 'TLV Properties bytes' response failed. Not enough bytes.");
    };
    check_enough_bytes(2);
    uint8_t tag    = *it++;
    uint8_t length = *it++;
    check_enough_bytes(length);

    // Ludovic Rousseau example seems to assume that properties length
    // can be either 1, 2 or 4.
    // See
    // anonscm.debian.org/viewvc/pcsclite/trunk/Drivers/ccid/examples/scardcontrol.c?view=markup
    EXCEPTION_ASSERT_WITH_LOG(length == 1 || length == 2 || length == 4,
                              LibLogicalAccessException,
                              "Unexpected length for TLV property")

    int32_t value;
    switch (length)
    {
    case 1:
        value = *it;
        break;
    case 2:
        value = *it + (*(it + 1) << 8);
        break;
    case 4:
        value = *it + (*(it + 1) << 8) + (*(it + 2) << 16) + (*(it + 3) << 24);
        break;
    }
    it += length;

    switch (tag)
    {
    case PCSCv2_PART10_PROPERTY_wLcdLayout:
        return Property(tag, value, "wLcdLayout");

    case PCSCv2_PART10_PROPERTY_bEntryValidationCondition:
        return Property(tag, value, "bEntryValidationCondition");

    case PCSCv2_PART10_PROPERTY_bTimeOut2:
        return Property(tag, value, "bTimeOut2");

    case PCSCv2_PART10_PROPERTY_wLcdMaxCharacters:
        return Property(tag, value, "wLcdMaxCharacters");

    case PCSCv2_PART10_PROPERTY_wLcdMaxLines:
        return Property(tag, value, "wLcdMaxLines");

    case PCSCv2_PART10_PROPERTY_bMinPINSize:
        return Property(tag, value, "bMinPINSize");

    case PCSCv2_PART10_PROPERTY_bMaxPINSize:
        return Property(tag, value, "bMaxPINSize");

    case PCSCv2_PART10_PROPERTY_sFirmwareID:
        return Property(tag, value, "sFirmwareID");

    case PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize:
        return Property(tag, value, "dwMaxAPDUDataSize");

    case PCSCv2_PART10_PROPERTY_bPPDUSupport:
        return Property(tag, value, "bPPDUSupport");

    case PCSCv2_PART10_PROPERTY_wIdVendor:
        return Property(tag, value, "wIdVendor");

    case PCSCv2_PART10_PROPERTY_wIdProduct:
        return Property(tag, value, "wIdProduct");
    }

    THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                             "Tag " + std::to_string(+tag) + " is not recognized");
}

std::pair<uint16_t, uint16_t> PCSCFeatures::getUSBInfo(
    const std::vector<PCSCFeatures::Property> &properties_vector)
{
    uint16_t vendor_id  = 0;
    uint16_t product_id = 0;

    for (const auto &p : properties_vector)
    {
        if (p.tag_ == PCSCv2_PART10_PROPERTY_wIdVendor)
            vendor_id = static_cast<uint16_t>(p.value_);
        else if (p.tag_ == PCSCv2_PART10_PROPERTY_wIdProduct)
            product_id = static_cast<uint16_t>(p.value_);
    }
    return std::make_pair(vendor_id, product_id);
}

std::pair<uint16_t, uint16_t> PCSCFeatures::getUSBInfo(PCSCConnection &connection)
{
    try
    {
        PCSCFeatures pcscf(&connection);
        return getUSBInfo(pcscf.getTLVProperties());
    }
    catch (const std::exception &e)
    {
        LOG(WARNINGS) << "Cannot retrieve USB information.";
        return std::make_pair(0, 0);
    }
}
