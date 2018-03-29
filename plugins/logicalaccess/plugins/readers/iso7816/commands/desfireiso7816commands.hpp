/**
 * \file desfireiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#ifndef LOGICALACCESS_DESFIREISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_DESFIREISO7816CARDPROVIDER_HPP

#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunit.hpp>
#include <logicalaccess/plugins/cards/samav2/samchip.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <logicalaccess/cards/samkeystorage.hpp>

namespace logicalaccess
{
#define DESFIRE_CLEAR_DATA_LENGTH_CHUNK 32
#define CMD_DESFIREISO7816 "DESFireISO7816"

/**
 * \brief The DESFire base commands class.
 */
class LIBLOGICALACCESS_API DESFireISO7816Commands : public DESFireCommands
{
  public:
    /**
     * \brief Constructor.
     */
    DESFireISO7816Commands();

    explicit DESFireISO7816Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~DESFireISO7816Commands();

    /**
     * Override setChip() so we can steal the DESFireProfile and forward
     * it to the crypto context.
     */
    void setChip(std::shared_ptr<Chip> chip) override;

    /**
     * \brief Erase the card.
     */
    void erase() override;

    /**
     * \brief Select an application.
     * \param aid The Application ID
     */
    void selectApplication(unsigned int aid) override;

    /**
     * \brief Create a new application.
     * \param aid The Application ID to create
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    void createApplication(unsigned int aid, DESFireKeySettings settings,
                           unsigned char maxNbKeys) override;

    /**
     * \brief Delete an application.
     * \param aid The Application ID to delete
     */
    void deleteApplication(unsigned int aid) override;

    /**
     * \brief Get the current application list.
     * \return The current application list.
     */
    std::vector<unsigned int> getApplicationIDs() override;

    /**
     * \brief Change a key in the current application.
     * \param keyno The key number to change
     * \param key The new key
     */
    void changeKey(unsigned char keyno, std::shared_ptr<DESFireKey> key) override;

    /**
     * \brief Get key settings on the current application.
     * \param settings Key settings
     * \param maxNbKeys Maximum number of keys
     */
    void getKeySettings(DESFireKeySettings &settings, unsigned char &maxNbKeys) override;

    /**
    * \brief Get a key version of a key
    * \param keyno The key no.
    */
    virtual uint8_t getKeyVersion(uint8_t keyno);

    /**
     * \brief Change key settings for the current application.
     * \param settings The new key settings
     */
    void changeKeySettings(DESFireKeySettings settings) override;

    /**
     * \brief Get the File IDentifiers of all active files within the currently selected
     * application
     * \return The file IDs.
     */
    ByteVector getFileIDs() override;

    /**
     * \brief Get settings of a specific file in the current application.
     * \param fileno The file number
     * \param fileSetting The file setting
     */
    FileSetting getFileSettings(unsigned char fileno) override;

    /**
     * \brief Change file settings of a specific file in the current application.
     * \param fileno The file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param plain Communication is currently in plain data.
     */
    void changeFileSettings(unsigned char fileno, EncryptionMode comSettings,
                            const DESFireAccessRights &accessRights, bool plain) override;

    /**
     * \brief Create a new data file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    void createStdDataFile(unsigned char fileno, EncryptionMode comSettings,
                           const DESFireAccessRights &accessRights,
                           unsigned int fileSize) override;

    /**
     * \brief Create a new backup file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     */
    void createBackupFile(unsigned char fileno, EncryptionMode comSettings,
                          const DESFireAccessRights &accessRights,
                          unsigned int fileSize) override;

    /**
     * \brief Create a new value file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param lowerLimit The file size (in bytes)
     * \param upperLimit The file size (in bytes)
     * \param value The default value
     * \param limitedCreditEnabled Set if the limited credit is enabled
     */
    void createValueFile(unsigned char fileno, EncryptionMode comSettings,
                         const DESFireAccessRights &accessRights, unsigned int lowerLimit,
                         unsigned int upperLimit, unsigned int value,
                         bool limitedCreditEnabled) override;

    /**
     * \brief Create a new linear record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     * \param maxNumberOfRecords Max number of records in the file.
     */
    void createLinearRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize,
                                unsigned int maxNumberOfRecords) override;

    /**
     * \brief Create a new cyclic record file in the current application.
     * \param fileno The new file number
     * \param comSettings The file communication setting
     * \param accessRights The file access rights
     * \param fileSize The file size.
     * \param maxNumberOfRecords Max number of records in the file.
     */
    void createCyclicRecordFile(unsigned char fileno, EncryptionMode comSettings,
                                const DESFireAccessRights &accessRights,
                                unsigned int fileSize,
                                unsigned int maxNumberOfRecords) override;

    /**
     * \brief Delete a file in the current application.
     * \param fileno The file number to delete
     */
    void deleteFile(unsigned char fileno) override;

    /**
     * \brief Read data from a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The data length to read
     * \param mode The communication mode
     * \return The bytes readed.
     */
    ByteVector readData(unsigned char fileno, unsigned int offset, unsigned int length,
                        EncryptionMode mode) override;

    /**
     * \brief Write data into a specific file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param data The data buffer
     * \param mode The communication mode
     */
    void writeData(unsigned char fileno, unsigned int offset, const ByteVector &data,
                   EncryptionMode mode) override;

    /**
     * \brief Get value from a specific value file.
     * \param handle The SCardHandle
     * \param fileno The file number
     * \param mode The communicatio mode
     * \param value The value stored in the card
     */
    void getValue(unsigned char fileno, EncryptionMode mode,
                  unsigned int &value) override;

    /**
     * \brief Credit a specific value file.
     * \param fileno The file number
     * \param value The value to credit
     * \param mode The communication mode
     */
    void credit(unsigned char fileno, unsigned int value, EncryptionMode mode) override;

    /**
     * \brief Debit a specific value file.
     * \param fileno The file number
     * \param value The value to debit
     * \param mode The communication mode
     */
    void debit(unsigned char fileno, unsigned int value, EncryptionMode mode) override;

    /**
     * \brief Limit credit on a specific value file.
     * \param handle The SCardHandle
     * \param fileno The file number
     * \param value The value to limit credit
     * \param mode The communication mode
     */
    void limitedCredit(unsigned char fileno, unsigned int value,
                       EncryptionMode mode) override;

    /**
     * \brief Write record into a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to write
     * \param data The data buffer
     * \param mode The communication mode
     */
    void writeRecord(unsigned char fileno, unsigned int offset, const ByteVector &data,
                     EncryptionMode mode) override;

    /**
     * \brief Read record from a specific record file.
     * \param fileno The file number
     * \param offset The byte offset
     * \param length The length to read
     * \param data The data buffer
     * \param mode The communication mode
     * \return The number of bytes read.
     */
    ByteVector readRecords(unsigned char fileno, unsigned int offset, unsigned int length,
                           EncryptionMode mode) override;

    /**
     * \brief Clear a specific record file.
     * \param fileno The file number.
     */
    void clearRecordFile(unsigned char fileno) override;

    /**
     * \brief Commit the transaction.
     */
    void commitTransaction() override;

    /**
     * \brief Abort the transaction.
     */
    void abortTransaction() override;

    /**
     * \brief Authenticate, given a key number.
     * \param keyno The key number, previously loaded on the profile.
     */
    void authenticate(unsigned char keyno = 0) override;

    /**
     * \brief Authenticate, given a key number.
     * \param keyno The key number.
     * \param key The key.
     */
    void authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key) override;

    void iks_des_authenticate(unsigned char keyno, std::shared_ptr<DESFireKey> key);

    /**
     * \brief Get the card version information.
     * \param dataVersion The card version information structure that will be filled
     */
    DESFireCardVersion getVersion() override;

    /**
     * \brief Get the ISO7816 reader/card adapter.
     * \return The ISO7816 reader/card adapter.
     */
    std::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
            getReaderCardAdapter());
    }

    /**
     * \brief Set the SAM Chip.
     * \param t The SAM Chip.
     */
    void setSAMChip(std::shared_ptr<SAMChip> t)
    {
        d_SAM_chip = t;
    }

    /**
     * \brief get the SAM Chip.
     */
    std::shared_ptr<SAMChip> getSAMChip() const
    {
        return d_SAM_chip;
    }

    /**
    * \brief retrieve key from SAM AV2 dump key.
    */
    void getKeyFromSAM(std::shared_ptr<DESFireKey> key, ByteVector diversify) const;

    ByteVector sam_authenticate_p1(std::shared_ptr<DESFireKey> key, ByteVector rndb,
                                   ByteVector diversify) const;

    void sam_authenticate_p2(unsigned char keyno, ByteVector rndap) const;

  protected:
    ByteVector getKeyInformations(std::shared_ptr<DESFireKey> key, uint8_t keyno) const;

    ByteVector getChangeKeySAMCryptogram(unsigned char keyno,
                                         std::shared_ptr<DESFireKey> key) const;

    ByteVector getChangeKeyIKSCryptogram(unsigned char keyno,
                                         std::shared_ptr<DESFireKey> key) const;

    /**
     * \brief Generic method to read data from a file.
     * \param err The last error code
     * \param firstMsg The first message to send
     * \param length The length to read
     * \param mode The communication mode
     * \return The data buffer.
     */
    virtual ByteVector handleReadData(unsigned char err, const ByteVector &firstMsg,
                                      unsigned int length, EncryptionMode mode);

    /**
     * \brief Generic method to write data into a file.
     * \param cmd The command to send
     * \param parameters The command parameters
     * \param data The data buffer
     * \param mode The communication mode
     */
    virtual void handleWriteData(unsigned char cmd, const ByteVector &parameters,
                                 const ByteVector &data, EncryptionMode mode);

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param lc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit(unsigned char cmd, unsigned char lc);

    /**
     * \brief Transmit a command.
     * \param cmd The command code.
     * \param buf The command buffer.
     * \param buflen The command buffer length.
     * \param forceLc Undocumented.
     * \return The response.
     */
    virtual ByteVector transmit(unsigned char cmd, const ByteVector &data = ByteVector(),
                                unsigned char lc = 0, bool forceLc = false);

    bool checkChangeKeySAMKeyStorage(unsigned char keyno,
                                     std::shared_ptr<DESFireKey> oldkey,
                                     std::shared_ptr<DESFireKey> key);

    /**
     * \brief The SAMChip used for the SAM Commands.
     */
    std::shared_ptr<SAMChip> d_SAM_chip;
};
}

#endif /* LOGICALACCESS_DESFIREISO7816COMMANDS_HPP */
