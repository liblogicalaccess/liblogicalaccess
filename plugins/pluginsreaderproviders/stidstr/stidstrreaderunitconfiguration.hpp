/**
 * \file stidstrreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR Reader unit configuration.
 */

#ifndef LOGICALACCESS_STIDSTRREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_STIDSTRREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"
#include "logicalaccess/cards/aes128key.hpp"
#include "logicalaccess/cards/hmac1key.hpp"

namespace logicalaccess
{
    /**
     * \brief The STidSRT communication type.
     */
    typedef enum {
        STID_RS232 = 0x00, /**< RS232 communication */
        STID_RS485 = 0x01 /**< RS485 communication */
    } STidCommunicationType;

    /**
     * \brief The STidSRT communication mode.
     */
    typedef enum {
        STID_CM_PLAIN = 0x00, /**< Plain communication */
        STID_CM_SIGNED = 0x01, /**< Signed communication */
        STID_CM_CIPHERED = 0x02, /**< Ciphered communication */
        STID_CM_CIPHERED_AND_SIGNED = STID_CM_CIPHERED | STID_CM_SIGNED, /**< Ciphered and signed communication */
        STID_CM_RESERVED = 0x04 /**< Reserved */
    } STidCommunicationMode;

    /**
     * \brief The STidSTR reader unit configuration base class.
     */
    class LIBLOGICALACCESS_API STidSTRReaderUnitConfiguration : public ReaderUnitConfiguration
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
        virtual void resetConfiguration();

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const;

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
         * \brief Get the key used for HMAC signature.
         * \return The key.
         */
        boost::shared_ptr<HMAC1Key> getHMACKey() const;

        /**
         * \brief Set the key used for HMAC signature.
         * \param key The key.
         */
        void setHMACKey(boost::shared_ptr<HMAC1Key> key);

        /**
         * \brief Get the key used for AES enciphering.
         * \return The key.
         */
        boost::shared_ptr<AES128Key> getAESKey() const;

        /**
         * \brief Set the key used for AES enciphering.
         * \param key The key.
         */
        void setAESKey(boost::shared_ptr<AES128Key> key);

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
         * \brief The key used for HMAC signature.
         */
        boost::shared_ptr<HMAC1Key> d_key_hmac;

        /**
         * \brief The key used for AES enciphering.
         */
        boost::shared_ptr<AES128Key> d_key_aes;
    };
}

#endif