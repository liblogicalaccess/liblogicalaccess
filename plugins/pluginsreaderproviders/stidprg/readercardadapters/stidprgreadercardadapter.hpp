#pragma once

#include "logicalaccess/cards/readercardadapter.hpp"

namespace logicalaccess
{
class STidPRGReaderCardAdapter : public ReaderCardAdapter
{

  public:
    virtual std::vector<unsigned char>
    adaptCommand(const std::vector<unsigned char> &command) override;

    virtual std::vector<unsigned char>
    adaptAnswer(const std::vector<unsigned char> &answer) override;

  protected:
    uint8_t compute_crc(const std::vector<uint8_t> &cmd);
};
}