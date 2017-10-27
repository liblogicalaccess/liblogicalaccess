#pragma once

#include "logicalaccess/readerproviders/readerprovider.hpp"

namespace logicalaccess
{
#define READER_STIDPRG "STidPRG"

class LIBLOGICALACCESS_API STidPRGReaderProvider : public ReaderProvider
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
