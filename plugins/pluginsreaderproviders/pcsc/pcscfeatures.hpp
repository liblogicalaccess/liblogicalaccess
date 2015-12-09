#pragma once

#include <logicalaccess/lla_fwd.hpp>
#include "pcsc_connection.hpp"

namespace logicalaccess
{

/**
 * This class helps dealing with PCSC features.
 *
 * This part of PC/SC let us query the reader for available features.
 * As a special case, this class also support extracting TLV Properties
 * from the reader.
 *
 * @warning Most old reader will not support the commands sent by this class.
 *          The caller is expected to wrap method invocation with try/catch block.
  */
class PCSCFeatures
{
  public:
    /**
     * Create a PCSCFeatures object that can extract features and TLV
     * properties from a PCSC reader.
     *
     * @param connection A raw pointer to a connection. This object doesn't
     * take ownership. Caller must take proper care that the connection outlives
     * the PCSCFeatures object.
     */
    PCSCFeatures(PCSCConnection *connection);

    /**
     * A PCSC "feature" as supported by the reader.
     */
    struct Feature
    {
        Feature(uint8_t tag, uint32_t ioctl, const std::string &name);
        Feature(const Feature &) = default;

        /**
         * The tag of the feature, as defined by PCSC.
         * Example: FEATURE_WRITE_DISPLAY
         */
        uint8_t tag_;

        /**
         * The "control code" to send in the SCardControl() call
         */
        uint32_t ioctl_;

        /**
         * The textual name of the feature.
         */
        std::string name_;
    };

    /**
     * A TLV property extracted from the reader.
     *
     * Property can be extracted only if the reader support the
     * FEATURE_GET_TLV_PROPERTIES.
     */
    struct Property
    {
        Property(uint8_t tag, uint32_t value, const std::string &name);
        Property(const Property &) = default;
        uint8_t tag_;
        uint32_t value_;
        std::string name_;
    };

    /**
     * Returns a vector of features supported by the readers.
     */
    std::vector<Feature> getFeatures();

    /**
     * Returns a vector of Property reported by the reader.
     *
     * This gives us some additional information about the reader. This
     * is not supported on all reader.
     */
    std::vector<Property> getTLVProperties();

    /**
     * Returns a pair of USB <vendor_id:product_id> information.
     *
     * This is simply an helper function that doesn't perform any interaction
     * with the reader. Instead, it search the properties_vector parameter.
     */
    static std::pair<uint16_t, uint16_t>
    getUSBInfo(const std::vector<Property> &properties_vector);

    /**
     * Fetch the USB <vendor_id:product_id> information.
     *
     * This is done by querying the reader, through the `connection`
     * object.
     *
     * Returns a pair of <0:0> on failure.
     */
    static std::pair<uint16_t, uint16_t> getUSBInfo(PCSCConnection &connection);

  private:
    /**
     * Fetch the TLV buffer describing available features.
     */
    ByteVector fetch_features_bytes();

    /**
     * Extract a feature description from the bytes vector describing
     * features.
     *
     * Advance the `it` iterator. Throw on error.
     */
    Feature extract_feature_from_bytes(ByteVector::iterator &it,
                                       const ByteVector::iterator &end);


    /**
     * Fetch the raw TLV properties bytes.
     *
     * @param tvl_properties_features The FEATURE_GET_TLV_PROPERTIES
     * so we can use its `ioctl_` field as the control code to fetch properties
     * bytes.
     */
    ByteVector fetch_tlv_properties_bytes(const Feature &tvl_properties_features);

    /**
     * Extract a Property description from the bytes vector describing
     * TLV properties.
     *
     * Advance the `it` iterator. Throw on error.
     */
    Property extract_property_from_bytes(ByteVector::iterator &it,
                                         const ByteVector::iterator &end);

    PCSCConnection *connection_;
};
}
