/**
 * \file scielreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default SCIEL reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/sciel/scielreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default SCIEL reader/card adapter class.
 */
class LLA_READERS_SCIEL_API SCIELReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     */
    SCIELReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~SCIELReaderCardAdapter();

    static const unsigned char STX; /**< \brief The STX value. */
    static const unsigned char ETX; /**< \brief The ETX value. */

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

    /**
     * \brief Wait for receiving all the tags list.
     * \param command The command requesting the tags list.
     * \param timeout The timeout value, in milliseconds. If timeout is negative, the call
     * never times out.
     * \return The list of tags detected by the reader.
     */
    std::list<ByteVector> receiveTagsListCommand(const ByteVector &command,
                                                 long int timeout = 2000);
};
}

#endif /* LOGICALACCESS_DEFAULTSCIELREADERCARDADAPTER_HPP */