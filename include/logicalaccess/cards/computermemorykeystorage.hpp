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
        * \brief Constructor.
        */
        ComputerMemoryKeyStorage();

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

        /**
        * \brief Set if the key is locked.
        * \param locked True if the key is locked, false otherwise.
        */
        void setLocked(bool locked);

        /**
        * \brief Get if the key is locked.
        * \return True if the key is locked, false otherwise.
        */
        bool getLocked() const;

        /**
        * \brief Set if the key is random.
        * \param random True if the key is random, false otherwise.
        */
        void setRandom(bool random);

        /**
        * \brief Get if the key is random.
        * \return True if the key is random, false otherwise.
        */
        bool getRandom() const;

    protected:

        /**
        * \brief True if the key is locked. If locked, the key shouldn't be changed or displayed without a complete erase.
        */
        bool d_locked;

        /**
        * \brief True if the key is random. If random the key is generated at each use.
        */
        bool d_random;
    };
}

#endif /* LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP */