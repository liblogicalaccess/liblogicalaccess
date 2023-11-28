/**
 * \file libusbreadercardadapter.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Default LibUSB reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTLIBUSBREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTLIBUSBREADERCARDADAPTER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderprovider.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default LibUSB reader/card adapter class.
 */
class LLA_READERS_LIBUSB_API LibUSBReaderCardAdapter : public ISO7816ReaderCardAdapter
{
  public:
    /**
     *\ brief Constructor.
     */
    LibUSBReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~LibUSBReaderCardAdapter();

    /**
     * \brief Adapt the command to send to the reader.
     * \param command The command to send.
     * \return The adapted command to send.
     */
    std::vector<unsigned char> adaptCommand(const std::vector<unsigned char> &command) override;

    /**
     * \brief Adapt the asnwer received from the reader.
     * \param answer The answer received.
     * \return The adapted answer received.
     */
    std::vector<unsigned char> adaptAnswer(const std::vector<unsigned char> &answer) override;

    /**
     * We override this because a 0 byte return is valid with libusb.
     */
    std::vector<unsigned char> sendCommand(const std::vector<unsigned char> &command,
                                           long timeout = 3000) override;
};
}

#endif /* LOGICALACCESS_DEFAULTLIBUSBREADERCARDADAPTER_HPP */
