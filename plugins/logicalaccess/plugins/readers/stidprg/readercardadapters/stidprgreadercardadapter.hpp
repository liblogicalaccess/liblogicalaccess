#pragma once

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/stidprg/lla_readers_stidprg_api.hpp>

namespace logicalaccess
{
class LLA_READERS_STIDPRG_API STidPRGReaderCardAdapter : public ReaderCardAdapter
{

  public:
    ByteVector adaptCommand(const ByteVector &command) override;

    ByteVector adaptAnswer(const ByteVector &answer) override;

  protected:
    static uint8_t compute_crc(const ByteVector &cmd);
};
}