/**
 * \file HMAC1Key.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HMAC1 key.
 */

#ifndef LOGICALACCESS_HMAC1KEY_H
#define LOGICALACCESS_HMAC1KEY_H

#include "logicalaccess/Key.h"


/**
 * \brief The default hmac-1 key size
 */
#define HMAC1_KEY_SIZE 10

namespace LOGICALACCESS
{
	/**
	 * \brief A hmac-1 key class.
	 */
	class LIBLOGICALACCESS_API HMAC1Key : public Key
	{
		public:

			/**
			 * \brief Build an empty 10-bytes hmac-1 key.
			 */
			HMAC1Key();

			/**
			 * \brief Build a hmac-1 key given a string representation of it.
			 * \param str The string representation.
			 */
			HMAC1Key(const std::string& str);

			/**
			 * \brief Build a hmac-1 key given a buffer.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 */
			HMAC1Key(const void* buf, size_t buflen);

			/**
			 * \brief Get the key length.
			 * \return The key length.
			 */
			inline size_t getLength() const { return HMAC1_KEY_SIZE; };

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
			unsigned char d_key[HMAC1_KEY_SIZE];
	};
}

#endif /* LOGICALACCESS_HMAC1KEY_H */

