/**
 * \file rplethreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Rpleth reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/rpleth/rplethreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default Rpleth reader/card adapter class.
 */
class LLA_READERS_RPLETH_API RplethReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     */
    RplethReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~RplethReaderCardAdapter();

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
* \brief Send command to the reader.
* \param data The command.
     * \param waitanswer If the command is waiting a answer.
     * \param timeout Time until stop to wait. If timeout is -1 (the default) we will load
     * the value from the Settings.
* \return The data received.
*/
    ByteVector sendRplethCommand(const ByteVector &data, bool waitanswer,
                                 long timeout = -1);
};
}

#endif /* LOGICALACCESS_DEFAULTRPLETHREADERCARDADAPTER_HPP */