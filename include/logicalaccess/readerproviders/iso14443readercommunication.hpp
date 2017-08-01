/**
 * \file iso14443readercommunication.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO14443-3 Reader communication.
 */

#ifndef LOGICALACCESS_ISO14443READERCOMMUNICATION_HPP
#define LOGICALACCESS_ISO14443READERCOMMUNICATION_HPP

#include "logicalaccess/readerproviders/iso14443areadercommunication.hpp"
#include "logicalaccess/readerproviders/iso14443breadercommunication.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief The reader communication mode.
     */
    typedef enum
    {
        IM_UNKNOWN = 0x00,
        IM_ISO14443A = 0x01,
        IM_ISO14443B = 0x02
    }ReaderCommunicationMode;

    /**
     * \brief A ISO 14443-3 reader communication base class.
     */
    class LIBLOGICALACCESS_API ISO14443ReaderCommunication : public ISO14443AReaderCommunication, public ISO14443BReaderCommunication
	{
    public:

        /**
         * \brief Constructor.
         */
        ISO14443ReaderCommunication();

        /**
         * \brief Send a REQ command from the PCD to the PICC.
         * \return The ATQ value.
         */
        virtual std::vector<unsigned char> request();

        /**
         * \brief Manage collision.
         * \return PICC serial number.
         */
        virtual std::vector<unsigned char> anticollision();

        /**
         * \brief Select a PICC.
         * \param uid The PICC serial number.
         */

        virtual void selectIso(const std::vector<unsigned char>& uid);

        /**
         * \brief Send a HLT command from the PCD to the PICC.
         */
        virtual void halt();

    protected:

        /**
         * \brief The current reader communication mode.
         */
        ReaderCommunicationMode d_rcMode;
    };
}

#endif /* LOGICALACCESS_ISO14443READERCOMMUNICATION_HPP */