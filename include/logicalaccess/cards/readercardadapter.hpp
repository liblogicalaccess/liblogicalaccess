/**
 * \file readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/card adapter.
 */

#ifndef LOGICALACCESS_READERCARDADAPTER_HPP
#define LOGICALACCESS_READERCARDADAPTER_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "logicalaccess/resultchecker.hpp"

namespace logicalaccess
{
    /**
     * \brief A reader/card adapter base class. It provide an abstraction layer between the card and the reader to send chip command.
     */
    class LIBLOGICALACCESS_API ReaderCardAdapter
    {
    public:

		virtual ~ReaderCardAdapter() {};

        /**
         * \brief Adapt the command to send to the reader.
         * \param command The command to send.
         * \return The adapted command to send.
         */
        virtual std::vector<unsigned char> adaptCommand(const std::vector<unsigned char>& command);

        /**
         * \brief Adapt the answer received from the reader.
         * \param answer The answer received.
         * \return The adapted answer received.
         */
        virtual std::vector<unsigned char> adaptAnswer(const std::vector<unsigned char>& answer);

		/**
		* \brief Get the data transport.
		* \return The data transport.
		*/
        std::shared_ptr<DataTransport> getDataTransport() const { return d_dataTransport; };

		/**
		* \brief Set the data transport.
		* \param unit The data transport.
		*/
        void setDataTransport(std::shared_ptr<DataTransport> dataTransport) { d_dataTransport = dataTransport; };

		/**
		* \brief Send a command to the reader.
		* \param command The command buffer.
		* \param timeout The command timeout.
		* \return the result of the command.
		*/
        virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long timeout = 3000);

		/**
		* \brief Get the result checker.
		* \return The result checker.
		*/
		virtual std::shared_ptr<ResultChecker> getResultChecker() const { return d_ResultChecker; };

		/**
		* \brief Set the result checker.
		* \param unit The result checker.
		*/
		virtual void setResultChecker(std::shared_ptr<ResultChecker> checker){ d_ResultChecker = checker; };

    protected:

		/**
		* \brief The data transport.
		*/
        std::shared_ptr<DataTransport> d_dataTransport;

		/**
		* \brief The result checker.
		*/
		std::shared_ptr<ResultChecker> d_ResultChecker;
    };
}

#endif /* LOGICALACCESS_READERCARDADAPTER_HPP */