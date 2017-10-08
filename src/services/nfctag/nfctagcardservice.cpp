/**
 * \file nfctagcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief NFC Tag Card service.
 */

#include "logicalaccess/services/nfctag/nfctagcardservice.hpp"

namespace logicalaccess
{
	NFCTagCardService::NFCTagCardService(std::shared_ptr<Chip> chip)
		: CardService(chip, CST_NFC_TAG)
    {
    }

	NFCTagCardService::~NFCTagCardService()
    {
    }

	void NFCTagCardService::writeNDEF(std::shared_ptr<NdefMessage> /*records*/)
	{
	}

	void NFCTagCardService::eraseNDEF()
	{
	}
}