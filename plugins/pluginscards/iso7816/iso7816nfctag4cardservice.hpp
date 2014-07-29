/**
 * \file desfirenfctag4cardservice.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief DESFire NFC Tag 4 card service.
 */

#ifndef LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP
#define LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "iso7816chip.hpp"


namespace logicalaccess
{
	/**
	 * \brief The DESFire storage card service bas class.
	 */
	class LIBLOGICALACCESS_API ISO7816NFCTag4CardService : public CardService
	{
		public:

			/**
			 * \brief Constructor.
			 * \param chip The chip.
			 */
			ISO7816NFCTag4CardService(boost::shared_ptr<Chip> chip) : CardService(chip) {};

			~ISO7816NFCTag4CardService() {};

			void writeCapabilityContainer(unsigned short isoFID = 0xe103, unsigned short isoFIDNDEFFile = 0xe104, unsigned short NDEFFileSize = 0xff);

			void writeNDEFFile(NdefMessage& records, unsigned short isoFIDNDEFFile = 0xe104);

			NdefMessage readNDEFFile(unsigned short isoFIDApplication = 0xe105, unsigned short isoFIDNDEFFile = 0xe104);

			virtual CardServiceType getServiceType() const { return CardServiceType::CST_NFC_TAG; };

		protected:

			boost::shared_ptr<ISO7816Chip> getISO7816Chip() { return boost::dynamic_pointer_cast<ISO7816Chip>(getChip()); };
	};
}

#endif /* LOGICALACCESS_ISO7816NFCTAG4CARDSERVICE_HPP */

