#include <reader.h>
#include <logicalaccess/utils.hpp>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "pcscfeatures.hpp"

using namespace logicalaccess;

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
