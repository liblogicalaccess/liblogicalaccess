/**
 * \file samkeystorage.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SAM key storage description.
 */

#ifndef LOGICALACCESS_SAMKEYSTORAGE_HPP
#define LOGICALACCESS_SAMKEYSTORAGE_HPP

#include "logicalaccess/cards/keystorage.hpp"

namespace logicalaccess
{
    /**
     * \brief A SAM key storage class.
     */
    class LIBLOGICALACCESS_API SAMKeyStorage : public KeyStorage
    {
    public:

        /**
         * \brief Constructor.
         */
        SAMKeyStorage();

        /**
         * \brief Get the key storage type.
         * \return The key storage type.
         */
        virtual KeyStorageType getType() const;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         * \return The XML string.
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

        unsigned char getKeySlot() const { return d_key_slot; };

        void setKeySlot(unsigned char key_slot) { d_key_slot = key_slot; };

    protected:

        /**
         * \brief The key slot. Optional for most PICC operation, automatically detected
         */
        unsigned char d_key_slot;
    };
}

#endif /* LOGICALACCESS_SAMKEYSTORAGE_HPP */