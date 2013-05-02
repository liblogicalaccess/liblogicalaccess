/**
 * \file MifarePlusKey.h
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus key.
 */

#ifndef LOGICALACCESS_MIFAREPLUSKEY_H
#define LOGICALACCESS_MIFAREPLUSKEY_H

#include "logicalaccess/Key.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A MifarePlus Key class.
	 */
	class LIBLOGICALACCESS_API MifarePlusKey : public Key
	{
		public:

			/**
			 * \brief Build an empty 6-bytes Mifare key.
			 */
			MifarePlusKey(size_t keySize);

			/**
			 * \brief Build a Mifare key given a string representation of it.
			 * \param str The string representation.
			 */
			MifarePlusKey(const std::string& str, size_t keySize);

			/**
			 * \brief Build a Mifare key given a buffer.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 */
			MifarePlusKey(const void* buf, size_t buflen, size_t keySize);			

			/**
			 * \brief Destructor for the key.
			 */
			~MifarePlusKey();

			/**
			 * \brief Clear the key.
			 */
			virtual void clear();

			/**
			 * \brief Get the key length.
			 * \return The key length.
			 */
			inline size_t getLength() const { return d_keySize; };

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
			* \brief Get the key in a buffer.
			*/
			virtual inline std::vector<unsigned char> getDataToBuffer() { return std::vector<unsigned char>(getData(), getData() + getLength()); };

		protected:

			/**
			 * \brief Initialize the key with size;
			 */
			void InitKey(size_t keySize);

			/**
			 * \brief The key bytes;
			 */
			unsigned char* d_key;

			/**
			* \brief The key size
			*/
			size_t d_keySize;
	};
}

#endif /* LOGICALACCESS_MIFAREPLUSKEY_H */
