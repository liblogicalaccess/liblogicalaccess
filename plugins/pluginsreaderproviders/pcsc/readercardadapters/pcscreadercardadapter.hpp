/**
 * \file pcscreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default PC/SC reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP

#include "readercardadapters/iso7816readercardadapter.hpp"
#include "../pcscreaderprovider.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief A default PC/SC reader/card adapter class.
     */
    class LIBLOGICALACCESS_API PCSCReaderCardAdapter : public ISO7816ReaderCardAdapter
    {
    public:

        /**
         *\ brief Constructor.
         */
        PCSCReaderCardAdapter();

        /**
         * \brief Destructor.
         */
        virtual ~PCSCReaderCardAdapter();

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
    };
}

#endif /* LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP */