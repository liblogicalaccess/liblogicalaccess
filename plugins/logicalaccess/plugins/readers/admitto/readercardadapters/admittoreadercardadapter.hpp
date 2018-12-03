/**
 * \file admittoreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Admitto reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTADMITTOREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTADMITTOREADERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/admitto/lla_readers_admitto_api.hpp>
#include <string>
#include <vector>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
/**
 * \brief A default Admitto reader/card adapter class.
 */
class LLA_READERS_ADMITTO_API AdmittoReaderCardAdapter : public ReaderCardAdapter
{
  public:
    static const unsigned char CR; /**< \brief The first stop byte. */
    static const unsigned char LF; /**< \brief The second stop byte. */

    /**
     * \brief Constructor.
     */
    AdmittoReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~AdmittoReaderCardAdapter();

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

#endif /* LOGICALACCESS_DEFAULTADMITTOREADERCARDADAPTER_HPP */