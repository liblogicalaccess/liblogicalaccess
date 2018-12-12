/**
 * \file iso7816iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#ifndef LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP
#define LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunit.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_ISO7816ISO7816 "ISO7816ISO7816"

#define ISO7816_CLA_ISO_COMPATIBLE 0x00
#define ISO7816_INS_APPEND_RECORD 0xE2
#define ISO7816_INS_GET_CHALLENGE 0x84
#define ISO7816_INS_READ_RECORDS 0xB2
#define ISO7816_INS_EXTERNAL_AUTHENTICATE 0x82
#define ISO7816_INS_INTERNAL_AUTHENTICATE 0x88
#define ISO7816_INS_GENERAL_AUTHENTICATE 0x87
#define ISO7816_INS_SELECT_FILE 0xA4
#define ISO7816_INS_READ_BINARY 0xB0
#define ISO7816_INS_WRITE_BINARY 0xD0
#define ISO7816_INS_UPDATE_BINARY 0xD6
#define ISO7816_INS_ERASE_BINARY 0x0E
#define ISO7816_INS_GET_RESPONSE 0xC0
#define ISO7816_INS_GET_DATA 0xCA
#define ISO7816_INS_GET_DATA_LIST 0xCB
#define ISO7816_INS_PUT_DATA 0xDA
#define ISO7816_INS_APPLICATION_MANAGEMENT_REQUEST 0x41
#define ISO7816_INS_REMOVE_APPLICATION 0xED

/**
 * \brief The ISO7816 commands base class.
 */
class LLA_READERS_ISO7816_API ISO7816ISO7816Commands : public ISO7816Commands
{
  public:
    /**
     * \brief Constructor.
     */
    ISO7816ISO7816Commands();

    explicit ISO7816ISO7816Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~ISO7816ISO7816Commands();

    /**
     * \brief Read binary data.
     * \param data The buffer that will contains data.
     * \param dataLength The buffer length.
     * \param offset The read offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    ByteVector readBinary(size_t length, size_t offset, unsigned short efid = 0) override;

    /**
     * \brief Write binary data.
     * \param data The buffer that contains data to write.
     * \param dataLength The buffer length.
     * \param offset The write offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void writeBinary(const ByteVector &data, size_t offset,
                     unsigned short efid = 0) override;

    /**
     * \brief Update binary data.
     * \param data The buffer that contains data to update.
     * \param dataLength The buffer length.
     * \param offset The update offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void updateBinary(const ByteVector &data, size_t offset,
                      unsigned short efid = 0) override;

    /**
     * \brief Erase binary data.
     * \param offset The erase offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void eraseBinary(size_t offset, unsigned short efid = 0) override;

    /**
     * \brief Get data.
     * \param data The buffer that will contains data.
     * \param dataObject The data object to get.
     * \param length The buffer length.
     * \return True on success, false otherwise.
     */
    ByteVector getData(unsigned short dataObject, size_t length) override;

	/**
     * \brief Get data.
     * \param data The data object list to get.
     * \param length The expected object length.
     * \return The data of the object.
     */
    ByteVector getDataList(const ByteVector &data, size_t length,
                           unsigned short efid = ISO_DATA_OBJECT_SELECTED) override;

    /**
     * \brief Put data.
     * \param data The buffer that contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     * \return True on success, false otherwise.
     */
    void putData(const ByteVector &data, unsigned short dataObject) override;

	/**
     * \brief Get response.
     * \param maxlength The maximum expected length.
     * \return The response data.
     */
    ByteVector getResponse(unsigned char maxlength = 0) override;

    /**
     * \brief Select a file.
     * \param p1 The parameter 1.
     * \param p2 The parameter 2.
     * \param data The data buffer.
     * \param datalen The buffer length.
     */
    void selectFile(unsigned char p1, unsigned char p2, const ByteVector &data) override;

    /**
     * \brief Read records.
     * \param fid The FID
     * \param start_record The start record (0 = read last written record)
     * \param record_number The number of records to read
     * \return The record(s) data
     */
    ByteVector readRecords(unsigned short fid = 0, unsigned char start_record = 0,
                           ISORecords record_number = ISO_RECORD_ONERECORD) override;

    /**
     * \brief Append a record to a file.
     * \param data The record data
     * \param fid The FID
     */
    void appendrecord(const ByteVector &data = ByteVector(),
                      unsigned short fid     = 0) override;

    /**
     * \brief Get the ISO challenge for authentication.
     * \param length The challenge length (8 = 2K3DES, 16 = 3K3DES and AES)
     * \return The ISO challenge.
     */
    ByteVector getChallenge(unsigned int length = 8) override;

    /**
     * \brief ISO external authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). false otherwise. \param
     * keyno The key number. \param data The data.
     */
    void externalAuthenticate(unsigned char algorithm = 0x00,
                              bool globalReference = false, unsigned char keyno = 0x00,
                              const ByteVector &data = ByteVector()) override;

	/**
     * \brief ISO external authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). false otherwise. \param
     * keyno The key number. \param data The data.
     * \param le Expected response length.
	 * \return The authentication response.
     */
    ByteVector externalAuthenticate(unsigned char algorithm,
                              bool globalReference, unsigned char keyno,
                              const ByteVector &data,
                              unsigned char le) override;

    /**
     * \brief ISO internal authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key).
	 * \param keyno The key number.
	 * \param RPCD2 The RPCD2.
	 * \param length The length.
	 * \return The cryptogram.
     */
    ByteVector internalAuthenticate(unsigned char algorithm = 0x00,
                                    bool globalReference    = false,
                                    unsigned char keyno     = 0x00,
                                    const ByteVector &RPCD2 = ByteVector(),
                                    unsigned int length     = 16) override;

	/**
     * \brief ISO general authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). false otherwise. \param
     * keyno The key number.
     * \param dataField The data.
     * \param le Expected response length.
     * \return The authentication response.
     */
    ByteVector generalAuthenticate(unsigned char algorithm, bool globalReference,
                                           unsigned char keyno,
                                           const ByteVector &dataField,
                                           unsigned char le) override;

	ByteVector generalAuthenticate_challenge(unsigned char algorithm,
                                                     bool globalReference,
                                                     unsigned char keyno) override;

    ByteVector generalAuthenticate_response(unsigned char algorithm,
                                                    bool globalReference,
                                                    unsigned char keyno,
                                                    const ByteVector &data) override;

	void removeApplication(
        const ByteVector &data,
                      ISORemoveApplicationP1 p1 =
                          P1_STATE_OPERATIONAL_OR_INITIALIZATION_TO_CREATION) override;

	ByteVector applicationManagementRequest(
            const ByteVector &data,
            ISOApplicationManagementRequestP1 p1 = P1_STATE_CREATION_TO_OPERATIONAL,
            unsigned char p2                     = 0x00) override;

    /**
     * \brief Get the ISO7816 reader/card adapter.
     * \return The ISO7816 reader/card adapter.
     */
    std::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
            getReaderCardAdapter());
    }

  protected:
    /**
     * \brief Set P1 and P2 parameters.
     * \param offset The starting offset.
     * \param efid The EF identifier.
     * \param p1 The parameter 1.
     * \param p2 The parameter 2.
     */
    static void setP1P2(size_t offset, unsigned short efid, unsigned char &p1, unsigned char &p2);
};
} // namespace logicalaccess

#endif