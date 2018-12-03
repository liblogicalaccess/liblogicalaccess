#pragma once

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/stidprg/lla_readers_stidprg_api.hpp>

namespace logicalaccess
{
class STidPRGReaderUnitConfiguration;

class LLA_READERS_STIDPRG_API STidPRGReaderUnit : public ReaderUnit
{
  public:
    STidPRGReaderUnit();

    void serialize(boost::property_tree::ptree &node) override;

    void unSerialize(boost::property_tree::ptree &node) override;

    bool waitInsertion(unsigned int maxwait) override;

    bool waitRemoval(unsigned int maxwait) override;

    bool isConnected() override;

    void setCardType(std::string cardType) override;

    std::shared_ptr<Chip> getSingleChip() override;

    std::vector<std::shared_ptr<Chip>> getChipList() override;

    bool connect() override;

    void disconnect() override;

    bool connectToReader() override;

    void disconnectFromReader() override;

    std::string getName() const override;

    std::string getReaderSerialNumber() override;

    std::shared_ptr<STidPRGReaderUnitConfiguration>
    getSTidPRGReaderUnitConfiguration() const;

    ByteVector readBlocks(uint8_t start, uint8_t end);

    bool login(const ByteVector &password);

    bool writePassword(const ByteVector &old, const ByteVector &new_pass);

    bool writeBlock(uint8_t start, uint8_t end, const ByteVector &data);

    bool format(uint8_t start, uint8_t end, const ByteVector &pwd);

    void ihm_control(uint8_t device, int duration);

    void select_chip_type();

  protected:
    struct BuzzerModeGuard;
    friend struct BuzzerModeGuard;
    /**
     * Build a chip object corresponding to the chip that
     * can be read by the reader.
     * If nothing is read by the reader, returns nullptr.
     *
     * @warn This function may block for up to 3 seconds !
     */
    std::shared_ptr<Chip> getCurrentChip();

    void toggleBuzzer(bool on);
};
}
