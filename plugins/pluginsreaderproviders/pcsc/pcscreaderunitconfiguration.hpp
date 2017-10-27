/**
 * \file pcscreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC Reader unit configuration.
 */

#ifndef LOGICALACCESS_PCSCREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_PCSCREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"
#include "../iso7816/iso7816readerunitconfiguration.hpp"

#define DEVICE_TYPE_SMARTCARD /**< \brief Set device type to smart card */

#if defined(__unix__)
#include <pcsclite.h>
#endif

#include <string.h>

#ifdef UNICODE
#undef UNICODE
#include <winscard.h>
#define UNICODE
#else
#include <winscard.h>
#endif

namespace logicalaccess
{
/**
 * \brief Share mode enumeration.
 */
typedef enum {
    SC_SHARED           = 2, // SCARD_SHARE_SHARED
    SC_SHARED_EXCLUSIVE = 1, // SCARD_SHARE_EXCLUSIVE
    SC_DIRECT           = 3  // SCARD_SHARE_DIRECT
} PCSCShareMode;

/**
 * Returns the share mode as text.
 */
std::string LIBLOGICALACCESS_API pcsc_share_mode_to_string(PCSCShareMode sm);

/**
 * Returns the pcsc protocol as text.
 */
std::string LIBLOGICALACCESS_API pcsc_protocol_to_string(unsigned long proto);

/**
 *\brief The reader unit types.
 */
typedef enum {
    PCSC_RUT_DEFAULT          = 0x0000, /**< Default PC/SC reader unit */
    PCSC_RUT_OMNIKEY_XX21     = 0x0001, /**< Omnikey xx21 PC/SC reader unit */
    PCSC_RUT_OMNIKEY_XX25     = 0x0002, /**< Omnikey xx25 PC/SC reader unit */
    PCSC_RUT_SCM              = 0x0003, /**< SCM PC/SC reader unit */
    PCSC_RUT_CHERRY           = 0x0004, /**< Cherry SmartTerminal PC/SC reader unit */
    PCSC_RUT_SPRINGCARD       = 0x0005, /**< SpringCard PC/SC reader unit */
    PCSC_RUT_OMNIKEY_XX27     = 0x0006, /**< Omnikey xx27 PC/SC reader unit */
    PCSC_RUT_OMNIKEY_LAN_XX21 = 0x0007, /**< Omnikey PC/SC LAN reader unit */
    PCSC_RUT_ACS_ACR          = 0x0008, /**< ACS ACR reader unit */
    PCSC_RUT_ACS_ACR_1222L    = 0x0009, /**< ACS ACR 1222L reader unit */
    PCSC_RUT_ID3_CL1356       = 0x000A, /**< ID3 (Orcanthus) CL1356A+ reader unit */
    PCSC_RUT_OMNIKEY_XX22     = 0x000B, /**< Omnikey xx22 PC/SC reader unit */
    PCSC_RUT_OMNIKEY_XX23     = 0x000C, /**< Omnikey xx23 PC/SC reader unit */
} PCSCReaderUnitType;

/**
 * \brief The PC/SC reader unit configuration base class.
 */
class LIBLOGICALACCESS_API PCSCReaderUnitConfiguration
    : public ISO7816ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    PCSCReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~PCSCReaderUnitConfiguration();

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
     * \brief Get the transmission protocol.
     * \return The transmission protocol.
     */
    unsigned int getTransmissionProtocol() const;

    /**
     * \brief Set the transmission protocol.
     * \param protocol The transmission protocol.
     */
    void setTransmissionProtocol(unsigned int protocol);

    /**
     * \brief Get the share mode.
     * \return The share mode.
     */
    PCSCShareMode getShareMode() const;

    /**
     * \brief Set the share mode.
     * \param share_mode The share mode.
     */
    void setShareMode(PCSCShareMode share_mode);

    /**
     * \brief Get the PC/SC reader unit configuration type.
     * \return The PC/SC reader unit configuration type.
     */
    virtual PCSCReaderUnitType getPCSCType() const;

  protected:
    /**
     * \brief The transmission protocol.
     */
    unsigned int d_protocol;

    /**
     * \brief The share mode used when connecting to a card.
     */
    PCSCShareMode d_share_mode;
};
}

#endif
