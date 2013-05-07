/**
 * \file SAMKeyStorage.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SAM key storage description.
 */

#ifndef LOGICALACCESS_SAMKEYSTORAGE_H
#define LOGICALACCESS_SAMKEYSTORAGE_H

#include "logicalaccess/Cards/KeyStorage.h"


namespace LOGICALACCESS
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
			 * \brief Set the key slot.
			 * \param key_slot The key slot.
			 */
			void setKeySlot(unsigned char key_slot);

			/**
			 * \brief Get the key slot.
			 * \return The key slot.
			 */
			unsigned char getKeySlot() const;

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
				
			/**
			 * \brief The key slot.
			 */
			unsigned char d_key_slot;
	};
}

#endif /* LOGICALACCESS_SAMKEYSTORAGE_H */

