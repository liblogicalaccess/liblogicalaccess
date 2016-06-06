/**
* \file nfctag2cardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag 2 card service.
*/

#ifndef LOGICALACCESS_NFCTAG2CARDSERVICE_HPP
#define LOGICALACCESS_NFCTAG2CARDSERVICE_HPP

#include "logicalaccess/services/nfctag/nfctagcardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"
#include "mifareultralightchip.hpp"

namespace logicalaccess
{
	/**
	* \brief The NFC Tag 2 storage card service base class.
	*/
	class LIBLOGICALACCESS_API NFCTag2CardService : public NFCTagCardService
	{
	public:

		/**
		* \brief Constructor.
		* \param chip The chip.
		*/
		NFCTag2CardService(std::shared_ptr<Chip> chip) : NFCTagCardService(chip) {};

		virtual ~NFCTag2CardService() {};

		virtual std::shared_ptr<logicalaccess::NdefMessage> readNDEF();

		virtual void writeNDEF(std::shared_ptr<logicalaccess::NdefMessage> records);

		virtual void eraseNDEF();

		void writeCapabilityContainer();

	protected:

		std::shared_ptr<MifareUltralightChip> getMifareUltralightChip() { return std::dynamic_pointer_cast<MifareUltralightChip>(getChip()); };
	};
}

#endif /* LOGICALACCESS_NFCTAG2CARDSERVICE_HPP */