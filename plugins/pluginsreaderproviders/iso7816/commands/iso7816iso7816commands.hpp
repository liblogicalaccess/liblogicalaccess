/**
 * \file iso7816iso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#ifndef LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP
#define LOGICALACCESS_ISO7816ISO7816COMMANDS_HPP

#include "iso7816/iso7816commands.hpp"
#include "../iso7816readerunit.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_ISO7816ISO7816 "ISO7816ISO7816"
/**
 * \brief The ISO7816 commands base class.
 */
class LIBLOGICALACCESS_API ISO7816ISO7816Commands : public ISO7816Commands
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
    ByteVector readBinary(size_t length, size_t offset, short efid = 0) override;

    /**
     * \brief Write binary data.
     * \param data The buffer that contains data to write.
     * \param dataLength The buffer length.
     * \param offset The write offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void writeBinary(const ByteVector &data, size_t offset, short efid = 0) override;

    /**
     * \brief Update binary data.
     * \param data The buffer that contains data to update.
     * \param dataLength The buffer length.
     * \param offset The update offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void updateBinary(const ByteVector &data, size_t offset, short efid = 0) override;

    /**
     * \brief Erase binary data.
     * \param offset The erase offset.
     * \param efid The EF identifier to set as current.
     * \return True on success, false otherwise.
     */
    void eraseBinary(size_t offset, short efid = 0) override;

    /**
     * \brief Get data.
     * \param data The buffer that will contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     * \return True on success, false otherwise.
     */
    ByteVector getData(size_t length, unsigned short dataObject) override;

    /**
     * \brief Put data.
     * \param data The buffer that contains data.
     * \param dataLength The buffer length.
     * \param dataObject The data object to get.
     * \return True on success, false otherwise.
     */
    void putData(const ByteVector &data, unsigned short dataObject) override;

    /**
     * \brief Select a file by the file identifier.
     * \param efid The file identifier.
     * \return True on success, false otherwise.
     */
    void selectFile(unsigned short efid) override;

    /**
     * \brief Select a file by the DF name.
     * \param dfname The DF name.
     * \param dfnamelen The DF name length.
     * \return True on success, false otherwise.
     */
    void selectFile(unsigned char *dfname, size_t dfnamelen) override;

    /**
     * \brief Select a file.
     * \param p1 The parameter 1.
     * \param p2 The parameter 2.
     * \param data The data buffer.
     * \param datalen The buffer length.
     */
    virtual void selectFile(unsigned char p1, unsigned char p2, unsigned char *data,
                            size_t datalen);

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
    static void setP1P2(size_t offset, short efid, unsigned char &p1, unsigned char &p2);
};
}

#endif