/**
 * \file stidstrreaderunitconfiguration.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief STidSTR Reader unit configuration.
 */

#ifndef LOGICALACCESS_STIDSTRREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_STIDSTRREADERUNITCONFIGURATION_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/cards/aes128key.hpp>
#include <logicalaccess/cards/hmac1key.hpp>
#include <logicalaccess/plugins/readers/stidstr/lla_readers_stidstr_api.hpp>

namespace logicalaccess
{
/**
 * \brief The STid communication type.
 */
typedef enum {
    STID_RS232 = 0x00, /**< RS232 communication */
    STID_RS485 = 0x01  /**< RS485 communication */
} STidCommunicationType;

/**
 * \brief The STid communication mode (SSCP v1).
 */
typedef enum {
    STID_CM_PLAIN    = 0x00, /**< Plain communication */
    STID_CM_SIGNED   = 0x01, /**< Signed communication */
    STID_CM_CIPHERED = 0x02, /**< Ciphered communication */
    STID_CM_CIPHERED_AND_SIGNED = STID_CM_CIPHERED | STID_CM_SIGNED, /**< Ciphered and signed communication */
    STID_CM_RESERVED = 0x04 /**< Reserved */
} STidCommunicationMode;

/**
 * \brief The STid protocol version.
 */
typedef enum {
    STID_SSCP_V1    = 0x00, /**< SSCP v1 */
    STID_SSCP_V2    = 0x01 /**< SSCP v2 */
} STidProtocolVersion;

/**
 * \brief The STidSTR reader unit configuration base class.
 */
class LLA_READERS_STIDSTR_API STidSTRReaderUnitConfiguration
    : public ISO7816ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    STidSTRReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~STidSTRReaderUnitConfiguration();

    /**
     * \brief Reset the configuration to default values
     */
    void resetConfiguration() override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Get the reader RS485 address.
     * \return The RS485 address.
     */
    unsigned char getRS485Address() const;

    /**
     * \brief Set the reader RS485 address.
     * \param address The RS485 address.
     */
    void setRS485Address(unsigned char address);

    /**
     * \brief Get communication type.
     * \return The communication type.
     */
    STidCommunicationType getCommunicationType() const;

    /**
     * \brief Set the communication type.
     * \param ctype The communication type.
     */
    void setCommunicationType(STidCommunicationType ctype);

    /**
     * \brief Get communication mode.
     * \return The communication mode.
     */
    STidCommunicationMode getCommunicationMode() const;

    /**
     * \brief Set the communication mode.
     * \param cmode The communication mode.
     */
    void setCommunicationMode(STidCommunicationMode cmode);

    /**
     * \brief Get protocol version.
     * \return The protocol version.
     */
    STidProtocolVersion getProtocolVersion() const;

    /**
     * \brief Set the protocol version.
     * \param ctype The  protocol version.
     */
    void setProtocolVersion(STidProtocolVersion protocol);

    /**
     * \brief Get the key used for HMAC signature.
     * \return The key.
     */
    std::shared_ptr<HMAC1Key> getHMACKey() const;

    /**
     * \brief Set the key used for HMAC signature.
     * \param key The key.
     */
    void setHMACKey(std::shared_ptr<HMAC1Key> key);

    /**
     * \brief Get the key used for AES enciphering.
     * \return The key.
     */
    std::shared_ptr<AES128Key> getAESKey() const;

    /**
     * \brief Set the key used for AES enciphering.
     * \param key The key.
     */
    void setAESKey(std::shared_ptr<AES128Key> key);

    bool getPN532Direct() const;

    void setPN532Direct(bool direct);

  protected:
    /**
     * \brief The reader RS485 address (if communication type RS485 used).
     */
    unsigned char d_rs485Address;

    /**
     * \brief The reader communication type.
     */
    STidCommunicationType d_communicationType;

    /**
     * \brief The reader communication mode (security objective).
     */
    STidCommunicationMode d_communicationMode;
    
    /**
     * \brief The reader protocol version.
     */
    STidProtocolVersion d_protocolVersion;

    /**
     * \brief The key used for HMAC signature.
     */
    std::shared_ptr<HMAC1Key> d_key_hmac;

    /**
     * \brief The key used for AES enciphering.
     */
    std::shared_ptr<AES128Key> d_key_aes;

    /**
    * \brief Direct communication with the internal PN532 component.
    */
    bool d_pn532_direct;
};
}

#endif