/**
 * \file pcscreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC Reader unit configuration.
 */

#ifndef LOGICALACCESS_ISO7816READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_ISO7816READERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
#define READER_ISO7816 "iso7816"

    class DESFireKey;

    /**
     * \brief The PC/SC reader unit configuration base class.
     */
    class LIBLOGICALACCESS_API ISO7816ReaderUnitConfiguration : public ReaderUnitConfiguration
    {
    public:

        /**
         * \brief Constructor.
         */
        ISO7816ReaderUnitConfiguration();

        /**
         * \brief Constructor.
         * \param rpt Reader provider type.
         */
        ISO7816ReaderUnitConfiguration(std::string rpt);

        /**
         * \brief Destructor.
         */
        virtual ~ISO7816ReaderUnitConfiguration();

        /**
         * \brief Reset the configuration to the default one.
         */
        virtual void resetConfiguration();

        /**
         * \brief Set the SAM type.
         * \param t The SAM type.
         */
        void setSAMType(std::string t);

        /**
         * \brief get the SAM type.
         */
        std::string getSAMType() const;

        /**
         * \brief Set the SAM reader name.
         * \param t The SAM reader name.
         */
        void setSAMReaderName(std::string t);

        /**
         * \brief get the SAM reader name.
         */
        std::string getSAMReaderName() const;

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
         * \brief Set the SAM Key and Keyno for check if the SAM is the SAM we are waiting and for AV2 enable communication
         */
        void setSAMUnlockKey(boost::shared_ptr<DESFireKey> key, unsigned char keyno) { d_sam_key_unlock = key; d_keyno_unlock = keyno; };

        /**
         * \brief Get SAM Security Check Key
         */
        boost::shared_ptr<DESFireKey> getSAMUnLockKey() const { return d_sam_key_unlock; };

        /**
         * \brief Get SAM Security Check KeyNo
         */
        unsigned char getSAMUnLockkeyNo() const { return d_keyno_unlock; };

    protected:

        /**
         * \brief The SAM type.
         */
        std::string d_sam_type;

        /**
         * \brief The SAM reader name.
         */
        std::string d_sam_reader_name;

        /**
         * \brief The SAM Key to see if it is the SAM we are waiting
         */
        boost::shared_ptr<DESFireKey> d_sam_key_unlock;

        /**
         * \brief The SAM Key to see if it is the SAM we are waiting
         */
        unsigned char d_keyno_unlock;
    };
}

#endif