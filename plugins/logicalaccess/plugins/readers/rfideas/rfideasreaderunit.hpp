/**
 * \file rfideasreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief RFIDeas Reader unit.
 */

#ifndef LOGICALACCESS_RFIDEASREADERUNIT_HPP
#define LOGICALACCESS_RFIDEASREADERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/rfideas/rfideasreaderunitconfiguration.hpp>

#ifdef _WINDOWS
#include <rfideas/pcProxAPI.h>
#endif

namespace logicalaccess
{
class Profile;
class RFIDeasReaderCardAdapter;
class RFIDeasReaderProvider;

typedef void (*setTagIdBitsLengthFct)(std::shared_ptr<Chip> *, unsigned int);

/**
 * \brief The RFIDeas reader unit class.
 */
class LLA_READERS_RFIDEAS_API RFIDeasReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Destructor.
     */
    virtual ~RFIDeasReaderUnit();

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
     * \brief Get the tag id.
     * \return The tag id.
     */
    ByteVector getTagId();

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

    /**
     * \brief Get the Deister reader unit configuration.
     * \return The Deister reader unit configuration.
     */
    std::shared_ptr<RFIDeasReaderUnitConfiguration> getRFIDeasConfiguration()
    {
        return std::dynamic_pointer_cast<RFIDeasReaderUnitConfiguration>(
            getConfiguration());
    }

    /**
     * \brief Get the Deister reader provider.
     * \return The Deister reader provider.
     */
    std::shared_ptr<RFIDeasReaderProvider> getRFIDeasReaderProvider() const;

    /**
     * \brief Get the reader unit singleton instance.
     * \return The reader unit instance.
     */
    static std::shared_ptr<RFIDeasReaderUnit> getSingletonInstance();

  protected:
    /**
     * \brief Constructor.
     */
    RFIDeasReaderUnit();

    /**
     * \brief Initialize external functions.
     */
    void initExternFct();

    /**
     * \brief Uninitialize external functions.
     */
    void uninitExternFct();

    /**
     * \brief Initialize reader connection (USB or Serial port).
     */
    void initReaderCnx();

    /**
     * \brief Uninitialize reader connection.
     */
    void uninitReaderCnx();

#ifdef _WINDOWS
    GetLibVersion fnGetLibVersion;
    USBConnect fnUSBConnect;
    ComConnect fnCOMConnect;
    ComDisconnect fnCOMDisconnect;
    SetComSrchRange fnSetComSrchRange;
    ReadCfg fnReadCfg;
    GetFlags fnGetFlags;
    SetFlags fnSetFlags;
    WriteCfg fnWriteCfg;
    GetLastLibErr fnGetLastLibErr;
    GetActiveID fnGetActiveID;
    GetActiveID32 fnGetActiveID32;
    USBDisconnect fnUSBDisconnect;
    GetDevCnt fnGetDevCnt;
    SetActDev fnSetActDev;
    SetLUID fnSetLUID;
    GetQueuedID fnGetQueuedID;
    GetQueuedID_index fnGetQueuedID_index;
    SetConnectProduct fnSetConnectProduct;
    SetDevTypeSrch fnSetDevTypeSrch;

    HMODULE hWejAPIDLL;
#endif

    long d_deviceId;
    bool isCOMConnection;

    unsigned int d_lastTagIdBitsLength;
};
}

#endif