/**
 * \file promagreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Promag reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    /**
     * \brief A default Promag reader/card adapter class.
     */
    class LIBLOGICALACCESS_API PromagReaderCardAdapter : public ReaderCardAdapter
    {
    public:

        static const unsigned char STX; /**< \brief The start byte. */
        static const unsigned char ESC; /**< \brief The escape byte. */
        static const unsigned char BEL; /**< \brief The button pressed byte. */
        static const unsigned char CR; /**< \brief The first stop byte. */
        static const unsigned char LF; /**< \brief The second stop byte. */

        /**
         * \brief Constructor.
         */
        PromagReaderCardAdapter();

        /**
         * \brief Destructor.
         */
        virtual ~PromagReaderCardAdapter();

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
    };
}

#endif /* LOGICALACCESS_DEFAULTPROMAGREADERCARDADAPTER_HPP */