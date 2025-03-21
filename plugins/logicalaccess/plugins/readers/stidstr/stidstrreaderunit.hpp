/**
 * \file stidstrreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief STidSTR Reader unit.
 */

#ifndef LOGICALACCESS_STIDSTRREADERUNIT_HPP
#define LOGICALACCESS_STIDSTRREADERUNIT_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816readerunit.hpp>
#include <logicalaccess/plugins/readers/stidstr/stidstr_fwd.hpp>
#include <logicalaccess/cards/chip.hpp>

namespace logicalaccess
{
/**
 * \brief The STid baudrates.
 */
typedef enum {
    STID_BR_9600   = 0x00, /**< 9600 baudrate */
    STID_BR_19200  = 0x01, /**< 19200 baudrate */
    STID_BR_38400  = 0x02, /**< 38400 baudrate */
    STID_BR_57600  = 0x03, /**< 57600 baudrate */
    STID_BR_115200 = 0x04  /**< 115200 baudrate */
} STidBaudrate;

/**
 * \brief The STid tamper switch behavior.
 */
typedef enum {
    STID_TS_NOTHING = 0x00, /**< Do nother when pullout */
    STID_TS_RESET_CONFIG =
        0x01, /**< Reset all configuration (baudrate, rs485 address, session keys, ...) */
    STID_TS_RESET_KEYS = 0x02, /**< Reset user keys */
    STID_TS_RESET_ALL  = 0x03  /**< Reset all configuration and user keys */
} STidTamperSwitchBehavior;

/**
 * \brief The STid key context.
 */
typedef enum {
    STID_KEYCTX_UNDEFINED = 0x00, /**< Undefined context */
    STID_KEYCTX_AUTH = 0x01, /** Authentication context */
    STID_KEYCTX_AUTH2 = 0x02, /** Authentication context with K2 */
    STID_KEYCTX_A_TO_B = 0x03, /**< Command from A to B */
    STID_KEYCTX_B_TO_A  = 0x04,  /**< Command from B to A */
    STID_KEYCTX_AES = 0x80 /** AES key type */
} STidKeyContext;

/**
 * \brief The STidSTR reader unit class.
 */
class LLA_READERS_STIDSTR_API STidSTRReaderUnit : public ISO7816ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    STidSTRReaderUnit();

    /**
     * \brief Destructor.
     */
    virtual ~STidSTRReaderUnit();

    /**
     * \brief Get the reader unit name.
     * \return The reader unit name.
     */
    std::string getName() const override;

    /**
     * \brief Get the connected reader unit name.
     * \return The connected reader unit name.
     */
    std::string getConnectedName() override;

    /**
     * \brief Set the card type.
     * \param cardType The card type.
     */
    void setCardType(std::string cardType) override;

    /**
     * \brief Wait for a card insertion.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was inserted, false otherwise. If a card was inserted, the
     * name of the reader on which the insertion was detected is accessible with
     * getReader().
     * \warning If the card is already connected, then the method always fail.
     */
    bool waitInsertion(unsigned int maxwait) override;

    /**
     * \brief Wait for a card removal.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was removed, false otherwise. If a card was removed, the
     * name of the reader on which the removal was detected is accessible with
     * getReader().
     */
    bool waitRemoval(unsigned int maxwait) override;

    /**
     * \brief Create the chip object from card type.
     * \param type The card type.
     * \return The chip.
     */
    std::shared_ptr<Chip> createChip(std::string type) override;

    /**
     * \brief Get the first and/or most accurate chip found.
     * \return The single chip.
     */
    std::shared_ptr<Chip> getSingleChip() override;

    /**
     * \brief Get chip available in the RFID rang.
     * \return The chip list.
     */
    std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * \brief Get the default STidSTR reader/card adapter.
     * \return The default STidSTR reader/card adapter.
     */
    virtual std::shared_ptr<STidSTRReaderCardAdapter>
    getDefaultSTidSTRReaderCardAdapter();

    /**
     * \brief Connect to the card.
     * \return True if the card was connected without error, false otherwise.
     *
     * If the card handle was already connected, connect() first call disconnect(). If you
     * intend to do a reconnection, call reconnect() instead.
     */
    bool connect() override;

    /**
     * \brief Disconnect from the Deister.
     * \see connect
     *
     * Calling this method on a disconnected Deister has no effect.
     */
    void disconnect() override;

    /**
     * \brief Check if the card is connected.
     * \return True if the card is connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Connect to the reader. Implicit connection on first command sent.
     * \return True if the connection successed.
     */
    bool connectToReader() override;

    /**
     * \brief Disconnect from reader.
     */
    void disconnectFromReader() override;

    /**
     * \brief Get the reader ping command.
     * \return The ping command.
     */
    ByteVector getPingCommand() const override;

    /**
     * \brief Get a string hexadecimal representation of the reader serial number
     * \return The reader serial number or an empty string on error.
     */
    std::string getReaderSerialNumber() override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    std::shared_ptr<STidSTRReaderUnitConfiguration> getSTidSTRConfiguration() const;

    /**
     * \brief Get the STidSTR reader provider.
     * \return The STidSTR reader provider.
     */
    std::shared_ptr<STidSTRReaderProvider> getSTidSTRReaderProvider() const;

    /**
     * \brief Scan the RFID field for ISO 14443-A tag detection.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scan14443A();

    /**
     * \brief Scan the RFID field for ISO 14443-A tag detection.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scanARaw();

    /**
     * \brief Scan the RFID field for ISO 14443-B tag detection.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scan14443B();

    /**
     * \brief Scan for virtual credential (BLE / NFC)
     * \param use_selected_key True to use key previously loaded with LoadKeyBlue_NFC + SelectKeyBlue_NFC, false to use key loaded by configuration with Load_Conf or LoadConf_xxxx.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scanBlueNFC(bool use_selected_key = false);

    /**
     * \brief Scan the RFID field for a tag detection.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scanGlobal();
    
    /**
     * \brief Scan the RFID field for a tag detection.
     * \return The chip object if a tag is inserted.
     */
    std::shared_ptr<Chip> scanGlobal(bool iso14443a, bool activeRats, bool iso14443b, bool lf125khz, bool blueNfc, bool selectedKeyBlueNfc, bool keyboard, bool imageScanEngine);

    /**
     * \brief Authenticate the host with the reader and genereate session keys for HMAC
     * and enciphering, 1/2. SSCP v1 only.
     * \param isHMAC True if the authentication process is for HMAC session key, false for
     * AES session key.
     * \return The reader response.
     */
    ByteVector authenticateReader1(bool isHMAC);

    /**
     * \brief Authenticate the host with the reader and genereate session keys for HMAC
     * and enciphering, 2/2. SSCP v1 only.
     * \param data The authentication command data.
     * \return The reader response.
     */
    ByteVector authenticateReader2(const ByteVector &data);

    /**
     * \brief Reset the reader authentication and session keys. Next frames will be
     * transmitted plain. SSCP v1 only.
     */
    void ResetAuthenticate();

    /**
     * \brief Change keys on the reader memory.
     * \param key_hmac The new HMAC user key. Empty to keep the current one.
     * \param key_hmac The new AES user key. Empty to keep the current one.
     */
    void ChangeReaderKeys(const ByteVector &key_hmac, const ByteVector &key_aes);

    /**
     * \brief Set the reader baudrate for serial communication.
     * \param baudrate The new baudrate to apply.
     */
    void setBaudRate(STidBaudrate baudrate);

    /**
     * \brief Set the RS485 reader address.
     * \param address The new reader address.
     */
    void set485Address(unsigned char address);

    /**
     * \brief STidSTR reader information.
     */
    struct STidSTRInformation
    {
        /**
         * \brief The reader version;
         */
        unsigned char version;

        /**
         * \brief The reader baudrate use for serial communication.
         */
        STidBaudrate baudrate;

        /**
         * \brief The RS485 reader address.
         */
        unsigned char rs485Address;

        /**
         * \brief The power voltage actually used on the reader.
         */
        float voltage;
    };

    /**
     * \brief Get the reader information.
     * \return The reader information.
     */
    STidSTRInformation getReaderInformaton();

    /**
     * \brief Set the allowed communication modes with the reader.
     * \param plainComm True to be allowed to use plain communication mode, false
     * otherwise.
     * \param signedComm True to be allowed to use signed communication mode without
     * ciphering, false otherwise.
     * \param cipheredComm True to be allowed to use ciphered communication mode without
     * signature, false otherwise.
     */
    void setAllowedCommModes(bool plainComm, bool signedComm, bool cipheredComm);

    /**
     * \brief Set the tamper switch settings.
     * \param useTamperSwitch True to use tamper switch action, false to not.
     * \param behavior The behavior when tamper switch is activated.
     */
    void setTamperSwitchSettings(bool useTamperSwitch, STidTamperSwitchBehavior behavior);

    /**
     * \brief Get the tamper switch settings.
     * \param useTamperSwitch True to use tamper switch action, false to not.
     * \param behavior The behavior when tamper switch is activated.
     * \param swChanged True if the SW state have changed, false otherwise.
     */
    void getTamperSwitchInfos(bool &useTamperSwitch, STidTamperSwitchBehavior &behavior,
                              bool &swChanged);

    /**
     * \brief Save in EEPROM INDEX table memory the keys from a SKB card.
     */
    void loadSKB();

    /**
     * \brief Calculate the message HMAC.
     * \param buf The message buffer.
     * \param kctx The key context.
     * \return The HMAC.
     */
    ByteVector calculateHMAC(const ByteVector &buf, STidKeyContext kctx) const;

    /**
     * \brief Cipher data.
     * \param buf The message buffer.
     * \param iv The initialization vector.
     * \param kctx The key context.
     * \return The ciphered data.
     */
    ByteVector cipherData(const ByteVector &buf, const ByteVector &iv, STidKeyContext kctx) const;

    /**
     * \brief Uncipher data.
     * \param buf The message buffer.
     * \param iv The initialization vector.
     * \param kctx The key context.
     * \return The unciphered data.
     */
    ByteVector uncipherData(const ByteVector &buf, const ByteVector &iv, STidKeyContext kctx) const;

    /**
     * \brief Get the command counter.
     * \param incr True to increment the counter after return, false otherwise.
     * \return The command counter.
     */
    int getCommandCounter(bool incr = false);

    /**
     * \brief Get the HMAC byte length.
     * \return The HMAC byte length.
     */
    unsigned int getHMACLength() const;

  protected:
    /**
     * \brief Get a key from context.
     * \param kctx The key context.
     * \return The key.
     */
    ByteVector getKeyFromContext(STidKeyContext kctx) const;

    /**
     * \brief Authenticate the host and the reader to obtain the HMAC session key. SSCP v1 only.
     */
    void authenticateHMAC();

    /**
     * \brief Authenticate the host and the reader to obtain the AES session key. SSCP v1 only.
     */
    void authenticateAES();

    /**
     * \brief Authenticate the host and the reader to obtain the session keys. SSCP v2 only.
     */
    void authenticateReader();
    
    std::shared_ptr<Chip> createChipFromiso14443aBuffer(const ByteVector& data);
    
    std::shared_ptr<Chip> createGenericChipFromBuffer(const ByteVector& data, std::string cardType = CHIP_GENERICTAG, bool lenIsShort = false);

    /**
     * \brief The HMAC session key, from A to B.
     */
    ByteVector d_sessionKey_hmac_ab;

    /**
     * \brief The HMAC session key, from B to A.
     */
    ByteVector d_sessionKey_hmac_ba;

    /**
     * \brief The AES session key, from A to B.
     */
    ByteVector d_sessionKey_aes_ab;

    /**
     * \brief The AES session key, from B to A.
     */
    ByteVector d_sessionKey_aes_ba;

    /**
     * \brief The command counter.
     */
    int d_cmd_counter;
};
}

#endif