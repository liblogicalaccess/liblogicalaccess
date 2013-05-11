/**
 * \file DESFireLocation.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireLocation.
 */

#ifndef LOGICALACCESS_DESFIRELOCATION_H
#define LOGICALACCESS_DESFIRELOCATION_H

#include "logicalaccess/Cards/Location.h"
#include "DESFireKey.h"

namespace logicalaccess
{
	/**
	 * \brief The file communication mode (or encryption mode)
	 */
	typedef enum
	{
		CM_PLAIN = 0x00,
		CM_MAC = 0x01,
		CM_ENCRYPT = 0x03,
		CM_UNKNOWN = 0xFF
	}EncryptionMode;

	/**
	 * \brief A DESFire location informations.
	 */
	class LIBLOGICALACCESS_API DESFireLocation : public Location
	{
		public:
			/**
			 * \brief Constructor.
			 */
			DESFireLocation();

			/**
			 * \brief Destructor.
			 */
			virtual ~DESFireLocation();

			/**
			 * \brief Convert an Application ID buffer into 32 bits.
			 * \param aid The Application ID buffer.
			 * \return The Application ID in 32 bits.
			 */
			static int convertAidToInt(const void* aid);

			/**
			 * \brief Convert an Application ID 32 bits into a buffer.
			 * \param i The Application ID in 32 bits.
			 * \param aid The Application ID buffer.
			 */
			static void convertIntToAid(int i, void* aid);
			
			/**
			 * \brief Export DESFire location informations to a buffer.
			 * \return The data.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import DESFire location informations from a buffer.
			 * \param data The buffer.
			 * \param offset An offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>&, size_t offset = 0);

			/**
			 * \brief DESFire location informations data size.
			 */
			virtual size_t getDataSize();

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
			 * \brief Get the card type for this location.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "DESFire"; }

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

		public:
			/**
			 * \brief The application ID.
			 */
			int aid;

			/**
			 * \brief The file ID.
			 */
			int file;

			/**
			 * \brief The byte offset.
			 */
			int byte;		

			/**
			 * \brief The file security level
			 */
			EncryptionMode securityLevel;
	};	
}

#endif /* LOGICALACCESS_DESFIRELOCATION_H */
