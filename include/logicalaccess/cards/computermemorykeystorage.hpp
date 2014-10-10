/**
 * \file computermemorykeystorage.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Computer memory key storage description.
 */

#ifndef LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP
#define LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP

#include "logicalaccess/cards/keystorage.hpp"

namespace logicalaccess
{
    /**
     * \brief A computer memory key storage class.
     */
    class LIBLOGICALACCESS_API ComputerMemoryKeyStorage : public KeyStorage
    {
    public:

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

    protected:
    };
}

#endif /* LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP */