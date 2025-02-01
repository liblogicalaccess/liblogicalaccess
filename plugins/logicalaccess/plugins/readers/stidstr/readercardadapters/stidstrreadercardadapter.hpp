/**
 * \file stidstrreadercardadapter.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Default STidSTR reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstrreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
/**
 * \brief The STid command type.
 */
typedef enum {
    STID_CMD_READER             = 0x00, /**< Reader command type */
    STID_CMD_MIFARE_CLASSIC     = 0x01, /**< Mifare classic command type */
    STID_CMD_DESFIRE            = 0x02, /**< DESFire command type */
    STID_CMD_MIFARE_PLUS        = 0x03, /**< Mifare plus command type */
    STID_CMD_RESERVED           = 0x04, /**< Reserved command type */
    STID_CMD_MIFARE_ULTRALIGHTC = 0x05, /**< Mifare ultralight C command type */
    STID_CMD_RFIDCHIP           = 0x06  /**< RFID chip command type */
} STidCmdType;

/**
 * \brief The STid protocol mode (SSCP v2).
 */
typedef enum {
    STID_PM_AUTH_REQUEST  = 0x00, /**< Authentication Request */
    STID_PM_NORMAL        = 0x01 /**< Normal Communication */
} STidProtocolMode;

/**
 * \brief A default STidSTR reader/card adapter class.
 */
class LLA_READERS_STIDSTR_API STidSTRReaderCardAdapter : public ISO7816ReaderCardAdapter
{
  public:
    /**
     * \brief Constructor.
     * \param adapterType The adapter type.
     * \param protocolMode The protocol mode.
     */
    explicit STidSTRReaderCardAdapter(STidProtocolMode protocolMode, STidCmdType adapterType = STID_CMD_READER);
                                      
    /**
     * \brief Constructor, for ISO-7816 command wrapping use.
     */
    explicit STidSTRReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~STidSTRReaderCardAdapter();

    static const unsigned char SOF; /**< \brief The start of message value. */

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
     * \brief Send a command to the reader.
     * \param commandCode The command code.
     * \param command The command buffer.
     * \param protocolMode Temporary override the default RCA protocol mode for this command.
     * \return The result of the command.
     */
    virtual ByteVector sendCommand(unsigned short commandCode, const ByteVector &command, STidProtocolMode protocolMode);

    /**
     * \brief Send a command to the reader.
     * \param commandCode The command code.
     * \param command The command buffer.
     * \param timeout The command timeout.
     * \return The result of the command.
     */
    virtual ByteVector sendCommand(unsigned short commandCode, const ByteVector &command,
                                   long int timeout = -1);

    /**
    * \brief Send a command to the reader.
    * \param command The command buffer.
    * \param timeout The command timeout.
    * \return the result of the command.
    */
    ByteVector sendCommand(const ByteVector &command, long timeout = -1) override;

  protected:
    std::shared_ptr<STidSTRReaderUnit> getSTidSTRReaderUnit() const;

    void signMessage(ByteVector& data) const;

    void unsignMessage(ByteVector& data) const;

    ByteVector cipherMessage(const ByteVector& data);

    ByteVector uncipherMessage(const ByteVector& data);

    /**
     * \brief Process message data to send.
     * \param commandCode The command code.
     * \param command The command message data.
     * \return The processed message (signed/ciphered).
     */
    ByteVector sendMessage(unsigned short commandCode, const ByteVector &command);

    /**
     * \brief Process message response to return plain message data and status code.
     * \param data The raw data from reader.
     * \param statusCode Will contains the response status code.
     * \return The plain message data.
     */
    ByteVector receiveMessage(const ByteVector &data, unsigned char &statusCode);

    /**
     * \brief Check status code and throw exception on error.
     * \param statusCode The status code.
     */
    void CheckError(unsigned char statusCode) const;

    /**
     * \brief Get the initialization vector for encryption.
     * \return The IV.
     * \remarks This behavior is a security threat into STid communication protocol !
     */
    ByteVector getIV();

    /**
     * \brief The adapter type.
     */
    STidCmdType d_adapterType;
    
    /**
     * \brief The protocol mode.
     */
    STidProtocolMode d_protocolMode;

    /**
    * \brief Use iso7816 / transparent mode.
    */
    bool d_iso7816;

    /**
     * \brief The last command code sent.
     */
    unsigned short d_lastCommandCode;

    /**
     * \brief The last IV to use.
     */
    ByteVector d_lastIV;
};
}

#endif /* LOGICALACCESS_DEFAULTSTIDSTRREADERCARDADAPTER_H */