/**
 * \file iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands. See
 * http://www.cardwerk.com/smartcards/smartcard_standard_ISO7816-4.aspx
 */

#ifndef LOGICALACCESS_ISO7816COMMANDS_HPP
#define LOGICALACCESS_ISO7816COMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816location.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/tlv.hpp>

namespace logicalaccess
{
#define CMD_ISO7816 "ISO7816"

/**
 * \brief The P1 of Select File command
 */
typedef enum
{
    P1_SELECT_MF_DF_EF                = 0x00,
    P1_SELECT_CHILD_DF                = 0x01,
    P1_SELECT_EF_UNDER_CURRENT_DF     = 0x02,
    P1_SELECT_PARENT_DF_OF_CURRENT_DF = 0x03,
    P1_SELECT_BY_DFNAME               = 0x04,
    P1_SELECT_PATH_FROM_MF            = 0x08,
    P1_SELECT_PATH_FROM_CURRENT_DF    = 0x09
} ISOSelectFileP1;

/**
 * \brief The P2 of Select File command
 */
typedef enum
{
    P2_FIRST_RECORD    = 0x00,
    P2_LAST_RECORD     = 0x01,
    P2_NEXT_RECORD     = 0x02,
    P2_PREVIOUS_RECORD = 0x03,
    // P2_RETURN_FCI		= 0x00,
    P2_RETURN_FCP    = 0x04,
    P2_RETURN_FMD    = 0x08,
    P2_RETURN_NO_FCI = 0x0C
} ISOSelectFileP2;

/**
 * \brief The P1 of Remove Application command
 */
typedef enum
{
    P1_STATE_CREATION_TO_APPLICATION_REMOVED           = 0x01,
    P1_STATE_INITIALIZATION_TO_CREATION                = 0x02,
    P1_STATE_INITIALIZATION_TO_APPLICATION_REMOVED     = 0x03,
    P1_STATE_OPERATIONAL_TO_CREATION                   = 0x04,
    P1_STATE_OPERATIONAL_OR_INITIALIZATION_TO_CREATION = 0x06,
    P1_STATE_OPERATIONAL_TO_APPLICATION_REMOVED        = 0x07
} ISORemoveApplicationP1;

/**
 * \brief The P1 of Application Management Request command
 */
typedef enum
{
    P1_STATE_NONEXISTENT_TO_CREATION       = 0x02,
    P1_STATE_CREATION_TO_INITIALIZATION    = 0x04,
    P1_STATE_NONEXISTENT_TO_INITIALIZATION = 0x06,
    P1_STATE_INITIALIZATION_TO_OPERATIONAL = 0x08,
    P1_STATE_CREATION_TO_OPERATIONAL       = 0x0C,
    P1_STATE_NONEXISTENT_TO_OPERATIONAL    = 0x0E
} ISOApplicationManagementRequestP1;

/**
 * \brief The ISO record transmission mode.
 */
typedef enum
{
    ISO_RECORD_ONERECORD  = 0x04,
    ISO_RECORD_ALLRECORDS = 0x05
} ISORecords;

#define ISO7816_DATA_OBJECT_SELECTED 0x3FFF
#define ISO7816_DATA_OBJECT_EXTENDED_HEADERLIST 0x4D
#define ISO7816_DATA_OBJECT_HEADERLIST 0x5C
#define ISO7816_DATA_OBJECT_TAGLIST 0x5D
#define ISO7816_DATA_OBJECT_APPLICATION_TEMPLATE 0x61
#define ISO7816_DATA_OBJECT_DYNAMIC_AUTHENTICATION_DATA 0x7C
#define ISO7816_DATA_OBJECT_WITNESS 0x80
#define ISO7816_DATA_OBJECT_CHALLENGE 0x81
#define ISO7816_DATA_OBJECT_RESPONSE 0x82
#define IS7816O_DATA_OBJECT_COMMITTED 0x83
#define ISO7816_DATA_OBJECT_AUTHCODE 0x84
#define ISO7816_DATA_OBJECT_CRYPTOGRAM 0x85
#define ISO7816_DATA_OBJECT_CRYPTOGRAPHIC_CHECKSUM 0x8E
#define ISO7816_DATA_OBJECT_PROTECTED_NE 0x97
#define ISO7816_DATA_OBJECT_PROCESSING_STATUS 0x99
#define ISO7816_DATA_OBJECT_IDENTIFICATION_DATA_TEMPLATE 0xA0

/**
 * \brief The ISO7816 commands class.
 */
class LLA_CARDS_ISO7816_API ISO7816Commands : public Commands
{
  public:
    ISO7816Commands()
        : Commands(CMD_ISO7816)
    {
    }

    explicit ISO7816Commands(std::string ct)
        : Commands(ct)
    {
    }

    virtual ~ISO7816Commands();

    /**
     * \brief Read binary data.
     * \param data The buffer that will contains data.
     * \param dataLength The buffer length.
     * \param offset The read offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    virtual ByteVector readBinary(size_t length, size_t offset,
                                  unsigned short efid = 0) = 0;

    /**
     * \brief Write binary data.
     * \param data The buffer that contains data to write.
     * \param dataLength The buffer length.
     * \param offset The write offset.
     * \param efid The EF identifier to set as current.
     */
    virtual void writeBinary(const ByteVector &data, size_t offset,
                             unsigned short efid = 0) = 0;

    /**
     * \brief Update binary data.
     * \param data The buffer that contains data to update.
     * \param dataLength The buffer length.
     * \param offset The update offset.
     * \param efid The EF identifier to set as current.
     */
    virtual void updateBinary(const ByteVector &data, size_t offset,
                              unsigned short efid = 0) = 0;

    /**
     * \brief Erase binary data.
     * \param offset The erase offset.
     * \param efid The EF identifier to set as current.
     */
    virtual void eraseBinary(size_t offset, unsigned short efid = 0) = 0;

    // Don't implement record management yet.

    /**
     * \brief Get data.
     * \param dataObject The data object to get.
     * \param length The expected object length.
     * \return The data of the object.
     */
    virtual ByteVector getData(unsigned short dataObject, size_t length = 0) = 0;

	/**
     * \brief Get data.
     * \param dataObject The data object to get.
     * \param listtag The list tag.
     * \param length The expected object length.
     * \return The data of the object.
     */
    virtual ByteVector getDataList(int64_t dataObject,
                                   unsigned char listtag = ISO7816_DATA_OBJECT_HEADERLIST,
                                   size_t length = 0, unsigned short efid = 0);

	/**
     * \brief Get data.
     * \param tlv The tlv with data object list to get.
     * \param length The expected object length.
     * \return The data of the object.
     */
    virtual ByteVector getDataList(TLVPtr tlv, size_t length = 0,
                                   unsigned short efid = ISO7816_DATA_OBJECT_SELECTED);

	/**
     * \brief Get data.
     * \param data The data object list to get.
     * \param length The expected object length.
     * \return The data of the object.
     */
    virtual ByteVector getDataList(const ByteVector &data, size_t length = 0,
                                   unsigned short efid = ISO7816_DATA_OBJECT_SELECTED) = 0;

    /**
     * \brief Put data.
     * \param data The buffer that contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     */
    virtual void putData(const ByteVector &data, unsigned short dataObject) = 0;

	/**
     * \brief Get response.
     * \param maxlength The maximum expected length.
     * \return The response data.
     */
    virtual ByteVector getResponse(unsigned char maxlength = 0) = 0;

    /**
     * \brief Select a file under current DF by the file identifier.
     * \param p1 The P1 parameter.
     * \param p2 The P2 parameter.
     * \param efid The file identifier.
     */
    virtual void selectFile(ISOSelectFileP1 p1, ISOSelectFileP2 p2, unsigned short efid);

    /**
     * \brief Select a file under current DF by the file identifier.
     * \param efid The file identifier.
     */
    virtual void selectFile(unsigned short efid);

    /**
     * \brief Select a file by the DF name.
     * \param dfname The DF name.
     */
    virtual void selectFile(const ByteVector &dfname);

    /**
     * \brief Select a file.
     * \param p1 The parameter 1.
     * \param p2 The parameter 2.
     * \param data The data buffer.
     */
    virtual void selectFile(unsigned char p1, unsigned char p2,
                            const ByteVector &data) = 0;

    /**
     * \brief Read records.
     * \param fid The FID
     * \param start_record The start record (0 = read last written record)
     * \param record_number The number of records to read
     * \return The record(s) data
     */
    virtual ByteVector readRecords(unsigned short fid = 0, unsigned char start_record = 0,
                                   ISORecords record_number = ISO_RECORD_ONERECORD) = 0;

    /**
     * \brief Append a record to a file.
     * \param data The record data
     * \param fid The FID
     */
    virtual void appendrecord(const ByteVector &data = ByteVector(),
                              unsigned short fid     = 0) = 0;

    /**
     * \brief Get the ISO challenge for authentication.
     * \param length The challenge length (8 = 2K3DES, 16 = 3K3DES and AES)
     * \return The ISO challenge.
     */
    virtual ByteVector getChallenge(unsigned int length = 8) = 0;

    /**
     * \brief ISO external authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). \param keyno The key
     * number. \param data The data.
     */
    virtual void externalAuthenticate(unsigned char algorithm = 0x00,
                                      bool globalReference    = false,
                                      unsigned char keyno     = 0x00,
                                      const ByteVector &data  = ByteVector()) = 0;

    /**
     * \brief ISO external authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). false otherwise. \param
     * keyno The key number. \param data The data.
     * \param le Expected response length.
     * \return The authentication response.
     */
    virtual ByteVector externalAuthenticate(unsigned char algorithm, bool globalReference,
                                            unsigned char keyno, const ByteVector &data,
                                            unsigned char le) = 0;

    /**
     * \brief ISO internal authenticate.
     * \param algorithm The ISO algorithm to use for authentication.
     * \param globalReference True if the referenced key to authenticate is global (e.g.
     * an MF secific key), false otherwise (e.g. DF specific key). \param keyno The key
     * number. \param RPCD2 The RPCD2. \param length The length. \return The cryptogram.
     */
    virtual ByteVector internalAuthenticate(unsigned char algorithm = 0x00,
                                            bool globalReference    = false,
                                            unsigned char keyno     = 0x00,
                                            const ByteVector &RPCD2 = ByteVector(),
                                            unsigned int length     = 16) = 0;

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
    virtual ByteVector generalAuthenticate(unsigned char algorithm, bool globalReference,
                                           unsigned char keyno,
                                           const ByteVector &dataField,
                                           unsigned char le) = 0;

	virtual ByteVector generalAuthenticate_challenge(unsigned char algorithm, bool globalReference,
                                           unsigned char keyno) = 0;

	virtual ByteVector generalAuthenticate_response(unsigned char algorithm,
                                               bool globalReference, unsigned char keyno,
                                               const ByteVector &data) = 0;

	virtual void removeApplication(const ByteVector &data,
                          ISORemoveApplicationP1 p1 =
                              P1_STATE_OPERATIONAL_OR_INITIALIZATION_TO_CREATION) = 0;

	virtual void removeApplication(TLVPtr tlv,
                               ISORemoveApplicationP1 p1 =
                                   P1_STATE_OPERATIONAL_OR_INITIALIZATION_TO_CREATION);

	virtual ByteVector applicationManagementRequest(
            const ByteVector &data,
            ISOApplicationManagementRequestP1 p1 = P1_STATE_CREATION_TO_OPERATIONAL,
            unsigned char p2                     = 0x00) = 0;

	virtual ByteVector applicationManagementRequest(
            TLVPtr tlv,
            ISOApplicationManagementRequestP1 p1 = P1_STATE_CREATION_TO_OPERATIONAL,
            unsigned char p2                     = 0x00);
};
} // namespace logicalaccess

#endif