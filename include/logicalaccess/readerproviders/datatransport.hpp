/**
 * \file datatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data transport for reader/card commands.
 */

#ifndef LOGICALACCESS_DATATRANSPORT_HPP
#define LOGICALACCESS_DATATRANSPORT_HPP

#include "logicalaccess/lla_fwd.hpp"
#include "logicalaccess/readerproviders/readerprovider.hpp"

#include <vector>

namespace logicalaccess
{
    /**
     * \brief A data transport base class. It provide an abstraction layer between the host and readers.
     */
    class LIBLOGICALACCESS_API DataTransport : public XmlSerializable
    {
    public:

        /**
         * \brief Get the reader unit.
         * \return The reader unit.
         */
        virtual std::shared_ptr<ReaderUnit> getReaderUnit() const { return d_ReaderUnit.lock(); }

        /**
         * \brief Set the reader unit.
         * \param unit The reader unit.
         */
        virtual void setReaderUnit(std::weak_ptr<ReaderUnit> unit){ d_ReaderUnit = unit; }

        /**
         * \brief Get the transport type of this instance.
         * \return The transport type.
         */
        virtual std::string getTransportType() const = 0;

        /**
         * \brief Connect to the transport layer.
         * \return True on success, false otherwise.
         */
        virtual bool connect() = 0;

        /**
         * \brief Disconnect from the transport layer.
         */
        virtual void disconnect() = 0;

        /**
         * \briaf Get if connected to the transport layer.
         * \return True if connected, false otherwise.
         */
        virtual bool isConnected() = 0;

        /**
         * \brief Get the data transport endpoint name.
         * \return The data transport endpoint name.
         */
        virtual std::string getName() const = 0;

        /**
         * \brief Send a command to the reader.
         * \param command The command buffer.
         * \param timeout The command timeout.
         * \return the result of the command.
         */
        virtual ByteVector sendCommand(const ByteVector& command, long int timeout = -1);

        /**
         * \brief Get the last command.
         * \return The last command.
         */
        virtual ByteVector getLastCommand() { return d_lastCommand; }

        /**
         * \brief Get the last command result.
         * \return The last command result.
         */
        virtual ByteVector getLastResult() { return d_lastResult; }

    protected:

        virtual void send(const ByteVector& data) = 0;

        virtual ByteVector receive(long int timeout) = 0;

        /**
         * \brief The reader unit.
         */
        std::weak_ptr<ReaderUnit> d_ReaderUnit;

        /**
         * \brief The last result.
         */
        ByteVector d_lastResult;

        /**
         * \brief The last command.
         */
        ByteVector d_lastCommand;
    };
}

#endif /* LOGICALACCESS_DATATRANSPORT_HPP */