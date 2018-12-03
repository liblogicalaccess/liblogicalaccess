#pragma once

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/stidprg/lla_readers_stidprg_api.hpp>

namespace logicalaccess
{
#define READER_STIDPRG "STidPRG"

class LLA_READERS_STIDPRG_API STidPRGReaderProvider : public ReaderProvider
{
  public:
    static std::shared_ptr<STidPRGReaderProvider> getSingletonInstance();
    std::shared_ptr<ReaderUnit> createReaderUnit() override;

    bool refreshReaderList() override;

    void release() override;

    const ReaderList &getReaderList() override;

    std::string getRPType() const override;

    std::string getRPName() const override;

  private:
    ReaderList d_readers;
};
}
