/**
 * \file tripledeskey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Triple DES key.
 */

#ifndef LOGICALACCESS_TRIPLEDESKEY_HPP
#define LOGICALACCESS_TRIPLEDESKEY_HPP

#include "logicalaccess/key.hpp"

/**
 * \brief The default triple des key size
 */
#define TRIPLEDES_KEY_SIZE 16

namespace logicalaccess
{
    /**
     * \brief A triple DES class.
     */
    class LIBLOGICALACCESS_API TripleDESKey : public Key
    {
    public:

        /**
         * \brief Build an empty 16-bytes triple DES key.
         */
        TripleDESKey();

        /**
         * \brief Build a triple DES key given a string representation of it.
         * \param str The string representation.
         */
        TripleDESKey(const std::string& str);

        /**
         * \brief Build a triple DES key given a buffer.
         * \param buf The buffer.
         * \param buflen The buffer length.
         */
        TripleDESKey(const void* buf, size_t buflen);

        /**
         * \brief Get the key length.
         * \return The key length.
         */
        inline size_t getLength() const { return TRIPLEDES_KEY_SIZE; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline const unsigned char* getData() const { return d_key; };

        /**
         * \brief Get the key data.
         * \return The key data.
         */
        inline unsigned char* getData() { return d_key; };

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

    private:

        /**
         * \brief The key bytes;
         */
        unsigned char d_key[TRIPLEDES_KEY_SIZE];
    };
}

#endif /* LOGICALACCESS_TRIPLEDESKEY_HPP */