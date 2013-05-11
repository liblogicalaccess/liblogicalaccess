/**
 * \file Wiegand37WithFacilityRightParity2Format.h
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand37 with facility and Right Parity 2 format class.
 */

#ifndef LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_H
#define LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_H	

#include "logicalaccess/Services/AccessControl/Formats/StaticFormat.h"

namespace logicalaccess
{
	/**
	 * \brief A Wiegand 37 with Facility Right Parity 2 format class.
	 */
	class LIBLOGICALACCESS_API Wiegand37WithFacilityRightParity2Format : public StaticFormat
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a Wiegand 37 With Facility Format Right Parity 2.
			 */
			Wiegand37WithFacilityRightParity2Format();

			/**
			 * \brief Destructor.
			 *
			 * Release the Wiegand 37 With Facility Format Right Parity 2.
			 */
			virtual ~Wiegand37WithFacilityRightParity2Format();

			/**
			 * \brief Get the format length in bits 
			 */
			virtual unsigned int getDataLength() const;

			/**
			 * \brief Get the format name
			 * \return The format name
			 */
			virtual string getName() const;

			/**
			 * \brief Get facility code.
			 */
			unsigned short int getFacilityCode() const;

			/**
			 * \brief Set facility code.
			 * \param facilityCode The facility code.
			 */
			void setFacilityCode(unsigned short int facilityCode);

			/**
			 * \brief Get the left parity for a buffer.
			 * \param data The buffer.
			 * \param dataLengthBytes The buffer length in bytes.
			 * \return The parity.
			 */
			unsigned char getLeftParity(const void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Get the right parity 1 for a buffer.
			 * \param data The buffer.
			 * \param dataLengthBytes The buffer length in bytes.
			 * \return The parity.
			 */
			unsigned char getRightParity1(const void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Get the right parity 2 for a buffer.
			 * \param data The buffer.
			 * \param dataLengthBytes The buffer length in bytes.
			 * \return The parity.
			 */
			unsigned char getRightParity2(const void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Get linear data.
			 * \param data Buffer to be modified. Should be allocated.
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual void getLinearData(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Set linear data.
			 * \param data Buffer where data will be written.
			 * \param dataLengthBytes Length of data in bytes
			 */
			virtual void setLinearData(const void* data, size_t dataLengthBytes);

			/**
			 * \brief Get the format linear data in bytes.
			 * \param data The data buffer
			 * \param dataLengthBytes The data buffer length
			 * \return The format linear data length.
			 */
			virtual size_t getFormatLinearData(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Set the format linear data in bytes.
			 * \param data The data buffer
			 * \param indexByte The data offset
			 */
			virtual void setFormatLinearData(const void* data, size_t* indexByte);

			/**
			 * \brief Get the format type.
			 * \return The format type.
			 */
			virtual FormatType getType() const;

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
			 * \brief Check the current format skeleton with another format.
			 * \param format The format to check.
			 * \return True on success, false otherwise.
			 */
			virtual bool checkSkeleton(boost::shared_ptr<Format> format) const;

		protected:

			struct {
				/**
				 * \brief The facility code.
				 */
				unsigned short int d_facilityCode;
			} d_formatLinear;
	};	
}

#endif /* LOGICALACCESS_WIEGAND37WITHFACILITYRP2FORMAT_H */
