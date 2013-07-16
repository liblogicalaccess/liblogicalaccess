/**
 * \file pcscreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC Reader unit configuration.
 */

#ifndef LOGICALACCESS_PCSCREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_PCSCREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"


#define DEVICE_TYPE_SMARTCARD /**< \brief Set device type to smart card */

#ifdef __linux__
#include <pcsclite.h>
#endif

#include <string.h>

#ifdef UNICODE
#undef UNICODE
#include <winscard.h>
#define UNICODE
#else
#include <winscard.h>
#endif

namespace logicalaccess
{
	/**
	 * \brief Share mode enumeration.
	 */
	typedef enum { SM_SHARED = SCARD_SHARE_SHARED, SM_EXCLUSIVE = SCARD_SHARE_EXCLUSIVE } PCSCShareMode;

	/**
	 *\brief The reader unit types.
	 */
	typedef enum {
		PCSC_RUT_DEFAULT = 0x0000, /**< Default PC/SC reader unit */
		PCSC_RUT_OMNIKEY_XX21 = 0x0001, /**< Omnikey PC/SC reader unit */
		PCSC_RUT_OMNIKEY_XX25 = 0x0002, /**< Omnikey PC/SC reader unit */
		PCSC_RUT_SCM_SDI010 = 0x0003, /**< SDI010 PC/SC reader unit */
		PCSC_RUT_CHERRY = 0x0004, /**< Cherry SmartTerminal PC/SC reader unit */
		PCSC_RUT_SPRINGCARD = 0x0005, /**< SpringCard PC/SC reader unit */
		PCSC_RUT_OMNIKEY_XX27 = 0x0006 /**< Omnikey PC/SC reader unit */
	} PCSCReaderUnitType;

	/**
	 *\brief The SAM unit types.
	 */
	typedef enum {
		SAM_AUTO = 0,
		SAM_NONE = 1,
		SAM_AV1 = 2,
		SAM_AV2 = 3
	} SAMType;

	/**
	 * \brief The PC/SC reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API PCSCReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:
			
			/**
			 * \brief Constructor.
			 */
			PCSCReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~PCSCReaderUnitConfiguration();

			/**
			 * \brief Reset the configuration to default values
			 */
			virtual void resetConfiguration();

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
			 * \brief Get the transmission protocol.
			 * \return The transmission protocol.
			 */
			unsigned int getTransmissionProtocol();

			/**
			 * \brief Set the transmission protocol.
			 * \param protocol The transmission protocol.
			 */
			void setTransmissionProtocol(unsigned int protocol);

			/**
			 * \brief Get the share mode.
			 * \return The share mode.
			 */
			PCSCShareMode getShareMode();

			/**
			 * \brief Set the share mode.
			 * \param share_mode The share mode.
			 */
			void setShareMode(PCSCShareMode share_mode);

			/**
			 * \brief Get the PC/SC reader unit configuration type.
			 * \return The PC/SC reader unit configuration type.
			 */
			virtual PCSCReaderUnitType getPCSCType() const { return PCSC_RUT_DEFAULT; };

			/**
			 * \brief Set the SAM type.
			 * \param t The SAM type.
			 */
			void setSAMType(SAMType t);
			
			/**
			 * \brief get the SAM type.
			 */
			SAMType getSAMType();

			/**
			 * \brief Set the SAM reader name.
			 * \param t The SAM reader name.
			 */
			void setSAMReaderName(std::string t);
			
			/**
			 * \brief get the SAM reader name.
			 */
			std::string getSAMReaderName();

		protected:

			/**
			 * \brief The transmission protocol.
			 */
			unsigned int d_protocol;

			/**
			 * \brief The share mode used when connecting to a card.
			 */
			PCSCShareMode d_share_mode;

			/**
			 * \brief The SAM type.
			 */
			SAMType d_sam_type;

			/**
			 * \brief The SAM reader name.
			 */
			std::string d_sam_reader_name;
	};
}

#endif
