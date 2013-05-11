/**
 * \file wiegand37withfacilityformat.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Wiegand37 with facility format class.
 */

#ifndef LOGICALACCESS_WIEGAND37WITHFACILITYFORMAT_HPP
#define LOGICALACCESS_WIEGAND37WITHFACILITYFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"

namespace logicalaccess
{
	/**
	 * \brief A Wiegand 37 with Facility format class.
	 */
	class LIBLOGICALACCESS_API Wiegand37WithFacilityFormat : public Wiegand37Format
	{
		public:
			/**
			 * \brief Constructor.
			 *
			 * Create a Wiegand 37 With Facility Format.
			 */
			Wiegand37WithFacilityFormat();

			/**
			 * \brief Destructor.
			 *
			 * Release the Wiegand 37 With Facility Format.
			 */
			virtual ~Wiegand37WithFacilityFormat();

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
			 * \brief Get linear data without parity
			 * \param data Buffer to be modified. Should be allocated.
			 * \param dataLengthBytes Length in byte of data
			 */
			virtual void getLinearDataWithoutParity(void* data, size_t dataLengthBytes) const;

			/**
			 * \brief Write linear data without parity into data buffer
			 * \param data Buffer where data will be written.
			 * \param dataLengthBytes Length of data in bytes
			 */
			virtual void setLinearDataWithoutParity(const void* data, size_t dataLengthBytes);

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
			 * \param xmlwriter The XML writer.
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

#endif /* LOGICALACCESS_WIEGAND37WITHFACILITYFORMAT_HPP */
