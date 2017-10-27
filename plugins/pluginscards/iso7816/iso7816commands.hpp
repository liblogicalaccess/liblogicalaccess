/**
 * \file iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands. See
 * http://www.cardwerk.com/smartcards/smartcard_standard_ISO7816-4.aspx
 */

#ifndef LOGICALACCESS_ISO7816COMMANDS_HPP
#define LOGICALACCESS_ISO7816COMMANDS_HPP

#include "logicalaccess/key.hpp"
#include "iso7816location.hpp"
#include "logicalaccess/cards/commands.hpp"

namespace logicalaccess
{
#define CMD_ISO7816 "ISO7816"

/**
 * \brief The ISO7816 commands class.
 */
class LIBLOGICALACCESS_API ISO7816Commands : public Commands
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

    /**
     * \brief Read binary data.
     * \param data The buffer that will contains data.
     * \param dataLength The buffer length.
     * \param offset The read offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    virtual ByteVector readBinary(size_t length, size_t offset, short efid = 0) = 0;

    /**
     * \brief Write binary data.
     * \param data The buffer that contains data to write.
     * \param dataLength The buffer length.
     * \param offset The write offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    virtual void writeBinary(const ByteVector &data, size_t offset, short efid = 0) = 0;

    /**
     * \brief Update binary data.
     * \param data The buffer that contains data to update.
     * \param dataLength The buffer length.
     * \param offset The update offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    virtual void updateBinary(const ByteVector &data, size_t offset, short efid = 0) = 0;

    /**
     * \brief Erase binary data.
     * \param offset The erase offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    virtual void eraseBinary(size_t offset, short efid = 0) = 0;

    // Don't implement record management yet.

    /**
     * \brief Get data.
     * \param data The buffer that will contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     * \return True on success, false otherwise.
     */
    virtual ByteVector getData(size_t length, unsigned short dataObject) = 0;

    /**
     * \brief Put data.
     * \param data The buffer that contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     * \return True on success, false otherwise.
     */
    virtual void putData(const ByteVector &data, unsigned short dataObject) = 0;

    /**
     * \brief Select a file by the file identifier.
     * \param efid The file identifier.
     * \return True on success, false otherwise.
     */
    virtual void selectFile(unsigned short efid) = 0;

    /**
     * \brief Select a file by the DF name.
     * \param dfname The DF name.
     * \param dfnamelen The DF name length.
     * \return True on success, false otherwise.
     */
    virtual void selectFile(unsigned char *dfname, size_t dfnamelen) = 0;

    // Don't implement security management yet.
};
}

#endif