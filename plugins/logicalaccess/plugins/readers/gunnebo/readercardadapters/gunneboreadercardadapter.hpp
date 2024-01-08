/**
 * \file gunneboreadercardadapter.h
 * \author Maxime C. <maxime@leosac.com>
 * \brief Default Gunnebo reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTGUNNEBOREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTGUNNEBOREADERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/gunnebo/gunneboreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
/**
 * \brief A default Gunnebo reader/card adapter class.
 */
class LLA_READERS_GUNNEBO_API GunneboReaderCardAdapter : public ReaderCardAdapter
{
  public:
    static const unsigned char STX; /**< \brief Start of TeXt. */
    static const unsigned char ETX; /**< \brief End of TeXt. */

    /**
     * \brief Constructor.
     */
    GunneboReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~GunneboReaderCardAdapter();

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

#endif /* LOGICALACCESS_DEFAULTGUNNEBOREADERCARDADAPTER_HPP */