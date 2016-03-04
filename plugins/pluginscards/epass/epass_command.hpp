#pragma once

#include "epass_crypto.hpp"
#include "utils.hpp"
#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{

class EPassCommand : public Commands
{
  public:
    EPassCommand();

    virtual void
    setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter) override;

    bool selectApplication(const ByteVector &app_id);
    bool selectEF(const ByteVector &file_id);

    bool selectIssuerApplication();
    bool authenticate(const std::string &mrz);

    /**
     * Read Binary of the currently selected file.
     */
    ByteVector readBinary(uint16_t offset, uint8_t length);

    /**
     * Retrieve the content of the EF.COM file.
     *
     * This is a necessary step because it enumerates the others files
     * present on the chip.
     */
    EPassEFCOM readEFCOM();

    /**
     * Fully read the current file.
     * @param size_bytes Is the number of bytes used to encode the size of the file,
     *        it will generally be 0 or 1.
     * @param size_offset The offset at which the bytes representing the data size
     * starts,
     *        this is generally 1-3, depending on the tag length.
     */
    ByteVector readEF(uint8_t size_bytes, uint8_t size_offset);

    /**
     * Extract information from Data Group 1.
     *
     * @note Data Group 1 contains the various data available on the MRZ.
     */
    EPassDG1 readDG1();

    /**
     * Retrieve the content of the Data Group 2 and present it
     * throughout a usable object.
     *
     * @note Data Group 2 contains facial biometric.
     */
    EPassDG2 readDG2();

    /**
     * Read the content of SOD, the area that contains hash value
     * for the rest of the EPassport. It allows for verifying the integrity
     * of the chip's content.
     */
    void readSOD();

  private:
    ByteVector compute_hash(const ByteVector &file_id);

    /**
     * Internal notification for when the crypto_ object
     * has changed.
     *
     */
    void cryptoChanged();
    std::shared_ptr<EPassCrypto> crypto_;
};
}
