/**
 * \file stidstrreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default STidSTR reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP

#include "iso7816/readercardadapters/iso7816readercardadapter.hpp"
#include "../stidstrreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    /**
     * \brief The STid command type.
     */
    typedef enum {
        STID_CMD_READER = 0x00, /**< Reader command type */
        STID_CMD_MIFARE_CLASSIC = 0x01, /**< Mifare classic command type */
        STID_CMD_DESFIRE = 0x02, /**< DESFire command type */
        STID_CMD_MIFARE_PLUS = 0x03, /**< Mifare plus command type */
        STID_CMD_RESERVED = 0x04, /**< Reserved command type */
        STID_CMD_MIFARE_ULTRALIGHTC = 0x05, /**< Mifare ultralight C command type */
        STID_CMD_RFIDCHIP = 0x06 /**< RFID chip command type */
    } STidCmdType;

    /**
     * \brief A default STidSTR reader/card adapter class.
     */
    class LIBLOGICALACCESS_API STidSTRReaderCardAdapter : public ISO7816ReaderCardAdapter
    {
    public:

        /**
         * \brief Constructor.
         * \param adapterType The adapter type.
         * \param iso7816 ISO7816 communication type (transparent mode)
         */
        STidSTRReaderCardAdapter(STidCmdType adapterType = STID_CMD_READER, bool iso7816 = false);

        /**
         * \brief Destructor.
         */
        virtual ~STidSTRReaderCardAdapter();

        static const unsigned char SOF; /**< \brief The start of message value. */

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
         * \param commandCode The command code.
         * \param command The command buffer.
         * \param timeout The command timeout.
         * \return The result of the command.
         */
        virtual std::vector<unsigned char> sendCommand(unsigned short commandCode, const std::vector<unsigned char>& command, long int timeout = -1);

        /**
        * \brief Send a command to the reader.
        * \param command The command buffer.
        * \param timeout The command timeout.
        * \return the result of the command.
        */
        virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long timeout = -1);

        /**
         * \brief Calculate the message HMAC.
         * \param buf The message buffer.
         * \return The HMAC.
         */
        std::vector<unsigned char> calculateHMAC(const std::vector<unsigned char>& buf) const;

    protected:

        std::shared_ptr<STidSTRReaderUnit> getSTidSTRReaderUnit() const;

        /**
         * \brief Process message data to send.
         * \param commandCode The command code.
         * \param command The command message data.
         * \return The processed message (signed/ciphered).
         */
        std::vector<unsigned char> sendMessage(unsigned short commandCode, const std::vector<unsigned char>& command);

        /**
         * \brief Process message response to return plain message data and status code.
         * \param data The raw data from reader.
         * \param statusCode Will contains the response status code.
         * \return The plain message data.
         */
        std::vector<unsigned char> receiveMessage(const std::vector<unsigned char>& data, unsigned char& statusCode);

        /**
         * \brief Check status code and throw exception on error.
         * \param statusCode The status code.
         */
        void CheckError(unsigned char statusCode) const;

        /**
         * \brief Get the initialization vector for encryption.
         * \return The IV.
         * \remarks This behavior is a security threat into STid communication protocol !
         */
        std::vector<unsigned char> getIV();

        /**
         * \brief The adapter type.
         */
        STidCmdType d_adapterType;

        /**
        * \brief Use iso7816 / transparent mode.
        */
        bool d_iso7816;

        /**
         * \brief The last command code sent.
         */
        unsigned short d_lastCommandCode;

        /**
         * \brief The last IV to use.
         */
        std::vector<unsigned char> d_lastIV;
    };
}

#endif /* LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_H */