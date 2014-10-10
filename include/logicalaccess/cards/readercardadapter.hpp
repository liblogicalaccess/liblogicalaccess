/**
 * \file readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/card adapter.
 */

#ifndef LOGICALACCESS_READERCARDADAPTER_HPP
#define LOGICALACCESS_READERCARDADAPTER_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"

namespace logicalaccess
{
    /**
     * \brief A reader/card adapter base class. It provide an abstraction layer between the card and the reader to send chip command.
     */
    class LIBLOGICALACCESS_API ReaderCardAdapter
    {
    public:

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

        boost::shared_ptr<DataTransport> getDataTransport() const { return d_dataTransport; };

        void setDataTransport(boost::shared_ptr<DataTransport> dataTransport) { d_dataTransport = dataTransport; };

        virtual std::vector<unsigned char> sendCommand(const std::vector<unsigned char>& command, long timeout = 3000);

    protected:

        boost::shared_ptr<DataTransport> d_dataTransport;
    };
}

#endif /* LOGICALACCESS_READERCARDADAPTER_HPP */