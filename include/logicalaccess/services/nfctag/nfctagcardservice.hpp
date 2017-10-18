/**
* \file nfctagcardservice.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief NFC Tag card service.
*/

#ifndef LOGICALACCESS_NFCTAGCARDSERVICE_HPP
#define LOGICALACCESS_NFCTAGCARDSERVICE_HPP

#include "logicalaccess/services/cardservice.hpp"
#include "logicalaccess/services/nfctag/ndefmessage.hpp"

namespace logicalaccess
{
    /**
    * \brief The NFC Tag storage card service base class.
    */
    class LIBLOGICALACCESS_API NFCTagCardService : public CardService
    {
    public:

        /**
        * \brief Constructor.
        * \param chip The chip.
        */
        explicit NFCTagCardService(std::shared_ptr<Chip> chip);

        virtual ~NFCTagCardService();

        virtual void writeNDEF(std::shared_ptr<NdefMessage> records);

        virtual std::shared_ptr<NdefMessage> readNDEF() = 0;

        virtual void eraseNDEF();
    };
}

#endif /* LOGICALACCESS_NFCTAGCARDSERVICE_HPP */