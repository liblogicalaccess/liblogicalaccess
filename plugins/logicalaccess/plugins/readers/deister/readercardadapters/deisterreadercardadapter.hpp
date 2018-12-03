/**
 * \file deisterreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Deister reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/deister/lla_readers_deister_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
/**
 * \brief A default Deister reader/card adapter class.
 */
class LLA_READERS_DEISTER_API DeisterReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     */
    DeisterReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~DeisterReaderCardAdapter();

    static const unsigned char SHFT;  /**< \brief The shift value. */
    static const unsigned char SOM;   /**< \brief The start of message value. */
    static const unsigned char SOC;   /**< \brief The start of command value. */
    static const unsigned char STOP;  /**< \brief The stop value, end of message. */
    static const unsigned char Dummy; /**< \brief The dummy value. */
    static const unsigned char
        SHFT_SHFT; /**< \brief The shift 00 value to replace SHFT value in message. */
    static const unsigned char
        SHFT_SOM; /**< \brief The shift 00 value to replace SOM value in message. */
    static const unsigned char
        SHFT_SOC; /**< \brief The shift 00 value to replace SOC value in message. */
    static const unsigned char
        SHFT_STOP; /**< \brief The shift 00 value to replace STOP value in message. */

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

  protected:
    /**
     * \brief Prepare data buffer for device.
     * \param data The data buffer to prepare.
     * \return The data buffer ready to send to device.
     */
    static ByteVector prepareDataForDevice(const ByteVector &data);

    /**
     * \brief Prepare data buffer from device.
     * \param data The data buffer to prepare.
     * \return The data buffer ready to use.
     */
    static ByteVector prepareDataFromDevice(const ByteVector &data);

    /**
     * \brief Target device bus address destination.
     */
    unsigned char d_destination;

    /**
     * \brief Transmitter bus address source.
     */
    unsigned char d_source;
};
}

#endif /* LOGICALACCESS_DEFAULTDEISTERREADERCARDADAPTER_HPP */