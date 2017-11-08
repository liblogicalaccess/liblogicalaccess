/**
 * \file ok5553readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default OK5553 reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTOK5553READERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTOK5553READERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/ok5553/ok5553readerunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default OK5553 reader/card adapter class.
 */
class LIBLOGICALACCESS_API OK5553ReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     */
    OK5553ReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~OK5553ReaderCardAdapter();

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

    ByteVector sendAsciiCommand(const std::string &command, long int timeout = 2000);
};
}

#endif /* LOGICALACCESS_DEFAULTOK5553READERCARDADAPTER_HPP */