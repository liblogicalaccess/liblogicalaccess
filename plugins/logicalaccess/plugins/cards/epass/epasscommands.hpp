#pragma once

#include <logicalaccess/plugins/cards/epass/epasscrypto.hpp>
#include <logicalaccess/plugins/cards/epass/utils.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>

namespace logicalaccess
{
#define CMD_EPASS "EPass"

class LLA_CARDS_EPASS_API EPassCommands : public Commands
{
  public:
    EPassCommands();

    explicit EPassCommands(std::string ct);

    void setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter) override;

    bool selectApplication(const ByteVector &app_id);

    bool selectIssuerApplication();
    bool authenticate(const std::string &mrz);

    /**
     * Retrieve the content of the EF.COM file.
     *
     * This is a necessary step because it enumerates the others files
     * present on the chip.
     */
    EPassEFCOM readEFCOM() const;

    /**
     * Fully read the current file.
     * @param size_bytes Is the number of bytes used to encode the size of the file,
     *        it will generally be 0 or 1.
     * @param size_offset The offset at which the bytes representing the data size
     * starts,
     *        this is generally 1-3, depending on the tag length.
     */
    ByteVector readEF(uint8_t size_bytes, uint8_t size_offset) const;

    /**
     * Extract information from Data Group 1.
     *
     * @note Data Group 1 contains the various data available on the MRZ.
     */
    EPassDG1 readDG1() const;

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
    void readSOD() const;

	virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const = 0;

  private:
    ByteVector compute_hash(const ByteVector &file_id) const;

    /**
     * The identifier of the currently selected application.
     *
     * This is a part of a hack. We ignore selectApplication if
     * the we already have selected the one we want. Somehow,
     * on some passport, this cause a failure.
     */
    ByteVector current_app_;

    std::shared_ptr<EPassCrypto> crypto_;
};
}
