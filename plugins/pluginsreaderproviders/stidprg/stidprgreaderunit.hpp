#pragma once

#include "logicalaccess/readerproviders/readerunit.hpp"

namespace logicalaccess
{
class STidPRGReaderUnitConfiguration;

class LIBLOGICALACCESS_API STidPRGReaderUnit : public ReaderUnit
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

    std::shared_ptr<STidPRGReaderUnitConfiguration> getSTidPRGReaderUnitConfiguration() const;

    std::vector<uint8_t> readBlocks(uint8_t start, uint8_t end);

    bool login(const std::vector<uint8_t> &password);

    bool writePassword(const std::vector<uint8_t> &old,
                       const std::vector<uint8_t> &new_pass);

    bool writeBlock(uint8_t start, uint8_t end, const std::vector<uint8_t> &data);

    bool format(uint8_t start, uint8_t end, const std::vector<uint8_t> &pwd);

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
