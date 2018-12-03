#pragma once

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/cards/epass/lla_cards_epass_api.hpp>

namespace logicalaccess
{
class EPassCrypto;
class LLA_CARDS_EPASS_API EPassReaderCardAdapter : public ISO7816ReaderCardAdapter
{

  public:
    EPassReaderCardAdapter();

    ByteVector adaptCommand(const ByteVector &command) override;

    ByteVector adaptAnswer(const ByteVector &answer) override;

    void setEPassCrypto(std::shared_ptr<EPassCrypto> crypto);

  private:
    /**
     * The cryptographic object that maintain the state.
     * It is used to encrypt/decrypt APDUs.
     */
    std::shared_ptr<EPassCrypto> crypto_;
};
}
