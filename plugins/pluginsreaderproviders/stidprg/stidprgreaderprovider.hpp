#pragma once

#include "logicalaccess/readerproviders/readerprovider.hpp"

namespace logicalaccess
{
#define READER_STIDPRG "STidPRG"

class LIBLOGICALACCESS_API STidPRGReaderProvider : public ReaderProvider
{
  public:
    static std::shared_ptr<STidPRGReaderProvider> getSingletonInstance();
    virtual std::shared_ptr<ReaderUnit> createReaderUnit() override;

    virtual bool refreshReaderList() override;

    virtual void release() override;

    virtual const ReaderList &getReaderList() override;

    virtual std::string getRPType() const override;

    virtual std::string getRPName() const override;

  private:
    ReaderList d_readers;
};
}
