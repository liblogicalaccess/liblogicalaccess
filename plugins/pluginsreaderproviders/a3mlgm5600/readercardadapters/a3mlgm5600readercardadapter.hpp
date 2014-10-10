/**
 * \file a3mlgm5600readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default A3MLGM5600 reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/readerproviders/iso14443readercommunication.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief A default A3MLGM5600 reader/card adapter class.
     */
    class LIBLOGICALACCESS_API A3MLGM5600ReaderCardAdapter : public ReaderCardAdapter, public ISO14443ReaderCommunication
    {
    public:

        /**
         * \brief Constructor.
         */
        A3MLGM5600ReaderCardAdapter();

        /**
         * \brief Destructor.
         */
        virtual ~A3MLGM5600ReaderCardAdapter();

        static const unsigned char STX = 0x02; /**< \brief The start value. */
        static const unsigned char ETX = 0x03; /**< \brief The stop value. */

        /**
         * \brief Adapt the command to send to the reader.
         * \param command The command to send.
         * \return The adapted command to send.
         */
        virtual std::vector<unsigned char> adaptCommand(const std::vector<unsigned char>& command);

        /**
         * \brief Adapt the asnwer received from the reader.
         * \param answer The answer received.
         * \return The adapted answer received.
         */
        virtual std::vector<unsigned char> adaptAnswer(const std::vector<unsigned char>& answer);

        /**
         * \brief Send a command to the reader.
         * \param cmd The command code.
         * \param data The command data buffer.
         * \param timeout The command timeout.
         * \return The result of the command.
         */
        virtual std::vector<unsigned char> sendCommand(unsigned char cmd, const std::vector<unsigned char>& data, long int timeout = 3000);

        /**
         * \brief Send a REQA command from the PCD to the PICC.
         * \return The ATQB PICC result.
         */
        virtual std::vector<unsigned char> requestA();

        /**
         * \brief Send a RATS command from the PCD to the PICC.
         * \return The ATS PICC result.
         */
        virtual std::vector<unsigned char> requestATS();

        /**
         * \brief Send a HLTB command from the PCD to the PICC.
         */
        virtual void haltA();

        /**
         * \brief Manage collision.
         * \return The chip UID.
         */
        virtual std::vector<unsigned char> anticollisionA();

        /**
         * \brief Send a REQB command from the PCD to the PICC.
         * \param afi The AFI value.
         * \return The ATQB PICC result.
         */
        virtual std::vector<unsigned char> requestB(unsigned char afi = 0x00);

        /**
         * \brief Send a HLTB command from the PCD to the PICC.
         */
        virtual void haltB();

        /**
         * \brief Send a attrib command from the PCD to the PICC.
         */
        virtual void attrib();

        /**
         * \brief Manage collision.
         * \param afi The AFI value.
         * \return The chip UID.
         */
        virtual std::vector<unsigned char> anticollisionB(unsigned char afi = 0x00);

    protected:

        /**
         * \brief Calculate command checksum.
         * \param data The data to calculate checksum
         * \return The checksum.
         */
        static unsigned char calcBCC(const std::vector<unsigned char>& data);

        /**
         * \brief The current sequence number.
         */
        unsigned char d_seq;

        /**
         * \brief The last command response status.
         */
        unsigned char d_command_status;
    };
}

#endif /* LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP */