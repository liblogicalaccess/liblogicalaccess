#pragma once

#include "iso7816/readercardadapters/iso7816readercardadapter.hpp"

namespace logicalaccess
{
class EPassCrypto;
class LIBLOGICALACCESS_API EPassReaderCardAdapter : public ISO7816ReaderCardAdapter
{

  public:
    EPassReaderCardAdapter();

  public:
    virtual ByteVector adaptCommand(const ByteVector &command) override;

    virtual ByteVector adaptAnswer(const ByteVector &answer) override;

    void setEPassCrypto(std::shared_ptr<EPassCrypto> crypto);

  private:
    /**
     * The cryptographic object that maintain the state.
     * It is used to encrypt/decrypt APDUs.
     */
    std::shared_ptr<EPassCrypto> crypto_;
};
}
