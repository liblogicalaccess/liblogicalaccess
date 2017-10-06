/**
 * \file iso7816ok5553readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 OK5553 reader/card adapter.
 */

#ifndef LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP

#include "ok5553readercardadapter.hpp"
#include "iso7816/readercardadapters/iso7816readercardadapter.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    /**
     * \brief A ISO7816 OK5553 reader/card adapter class.
     */
    class LIBLOGICALACCESS_API ISO7816OK5553ReaderCardAdapter : public ISO7816ReaderCardAdapter
    {
    public:

        /**
         * \brief Constructor.
         */
        ISO7816OK5553ReaderCardAdapter();

        /**
         * \brief Destructor.
         */
        virtual ~ISO7816OK5553ReaderCardAdapter();

        /**
         * \brief Send an APDU command to the reader.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char le);

        /**
         * \brief Send an APDU command to the reader.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen);

        /**
         * \brief Send an APDU command to the reader.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, const unsigned char* data, size_t datalen);
        /**
         * \brief Send an APDU command to the reader without data and result.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le);

        /**
         * \brief Send an APDU command to the reader without data.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, unsigned char le);

        /**
         * \brief Send an APDU command to the reader without data.
         */
        virtual std::vector<unsigned char> sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2);

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

    protected:

        /**
         * \brief Reader card adapter used to send command..
         */
        std::shared_ptr<OK5553ReaderCardAdapter> d_ok5553_reader_card_adapter;

        /**
         * \brief Handle the asnwer received from the reader.
         * \param answer The answer received.
         * \return The handled answer.
         */
        std::vector<unsigned char> handleAnswer(const std::vector<unsigned char>& answer);

        /**
         * \brief Reverse the asnwer received from the reader.
         * \param answer The answer received.
         * \return The reversed answer.
         */
        std::vector<unsigned char> answerReverse(const std::vector<unsigned char>& answer);

        /**
         * \brief Represent stat of prefix in desfire command. (0x02 of 0x03).
         */
        bool d_prefix;
    };
}

#endif /* LOGICALACCESS_ISO7816OK5553READERCARDADAPTER_HPP */