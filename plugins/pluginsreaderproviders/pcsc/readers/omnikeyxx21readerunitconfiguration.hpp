/**
 * \file omnikeyxx21readerunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 Reader unit configuration.
 */

#ifndef LOGICALACCESS_OMNIKEYXX21READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_OMNIKEYXX21READERUNITCONFIGURATION_HPP

#include "../pcscreaderunitconfiguration.hpp"
#include "logicalaccess/cards/tripledeskey.hpp"

namespace logicalaccess
{
    /**
     * \brief HID iClass data encryption mode for the secure mode.
     */
    typedef enum
    {
        HID_PLAIN = 0x00,
        HID_DES = 0x40,
        HID_3DES = 0x80,
        HID_AUTO = 0xc0
    } HIDEncryptionMode;

    /**
     * \brief The Omnikey XX21 reader unit configuration base class.
     */
    class LIBLOGICALACCESS_API OmnikeyXX21ReaderUnitConfiguration : public PCSCReaderUnitConfiguration
    {
    public:

        /**
         * \brief Constructor.
         */
        OmnikeyXX21ReaderUnitConfiguration();

        /**
         * \brief Destructor.
         */
        virtual ~OmnikeyXX21ReaderUnitConfiguration();

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
         * \brief Get the PC/SC reader unit configuration type.
         * \return The PC/SC reader unit configuration type.
         */
        virtual PCSCReaderUnitType getPCSCType() const { return PCSC_RUT_OMNIKEY_XX21; };

        /**
         * \brief Get if the secure mode is used.
         * \return The secure mode value, true if used, false otherwise.
         */
        bool getUseSecureMode() const;

        /**
         * \brief Set if the secure mode is used.
         * \param use_sm The secure mode value, true if used, false otherwise.
         */
        void setUseSecureMode(bool use_sm);

        /**
         * \brief Get the encryption mode.
         * \return The encryption mode.
         */
        HIDEncryptionMode getEncryptionMode() const;

        /**
         * \brief Set the encryption mode.
         * \param encryption_mode The encryption mode.
         */
        void setEncryptionMode(HIDEncryptionMode encryption_mode);

        /**
         * \brief Get the read key used in secure mode.
         * \return The read key.
         */
        std::shared_ptr<TripleDESKey> getSecureReadKey() const;

        /**
         * \brief Set the read key used in secure mode.
         * \param key The read key.
         */
        void setSecureReadKey(std::shared_ptr<TripleDESKey> key);

        /**
         * \brief Get the write key used in secure mode.
         * \return The write key.
         */
        std::shared_ptr<TripleDESKey> getSecureWriteKey() const;

        /**
         * \brief Set the read key used in secure mode.
         * \param key The read key.
         */
        void setSecureWriteKey(std::shared_ptr<TripleDESKey> key);

    protected:

        /**
         * \brief Set if use secure mode.
         */
        bool d_useSecureMode;

        /**
         * \brief The custom read key secure mode HID iClass (optional).
         */
        std::shared_ptr<TripleDESKey> d_secureReadKey;

        /**
         * \brief The custom write key secure mode HID iClass (optional).
         */
        std::shared_ptr<TripleDESKey> d_secureWriteKey;

        /**
         * \brief The encryption mode.
         */
        HIDEncryptionMode d_encryptionMode;
    };
}

#endif