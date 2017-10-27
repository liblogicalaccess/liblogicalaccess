/**
 * \file gigatmsreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default GIGA-TMS reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTGIGATMSREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTGIGATMSREADERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
/**
 * \brief A default GIGA-TMS reader/card adapter class.
 */
class LIBLOGICALACCESS_API GigaTMSReaderCardAdapter : public ReaderCardAdapter
{
  public:
    static const unsigned char STX1; /**< \brief The 1st start byte. */
    static const unsigned char STX2; /**< \brief The 2nd start byte. */

    /**
     * \brief Constructor.
     */
    GigaTMSReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~GigaTMSReaderCardAdapter();

    /**
     * \brief Adapt the command to send to the reader.
     * \param command The command to send.
     * \return The adapted command to send.
     */
    ByteVector adaptCommand(const ByteVector &command) override;

    /**
     * \brief Adapt the asnwer received from the reader.
     * \param answer The answer received.
     * \return The adapted answer received.
     */
    ByteVector adaptAnswer(const ByteVector &answer) override;
};
}

#endif /* LOGICALACCESS_DEFAULTGIGATMSREADERCARDADAPTER_HPP */