//
// Created by xaqq on 13/01/2022.
//

#include "samav3iso7816commands.hpp"
#include "samiso7816resultchecker.hpp"

namespace logicalaccess
{
void SAMAV3ISO7816Commands::yes_swig_we_are_abstract() {}

SAMAV3ISO7816Commands::SAMAV3ISO7816Commands()
    : SAMAV2ISO7816Commands(CMD_SAMAV3ISO7816)
{
}

std::vector<uint8_t> SAMAV3ISO7816Commands::getRandom(uint8_t random_size)
{
    unsigned char cmd[] = {d_cla, 0x84, 0x00, 0x00, random_size};
    ByteVector cmd_vector(cmd, cmd + 5);

    ByteVector result = transmit(cmd_vector);
    // todo remove ISO code.
    return result;
}

bool SAMAV3ISO7816Commands::ext_IsActivated()
{
    auto version = getVersion();
    if (version.manufacture.modecompatibility == 0xA3)
        return true;
    else if (version.manufacture.modecompatibility == 0x03)
        return false;
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            CardException,
            "Invalid flag for state/modecompat to detect if AV3 is activated or not");
    }
}

class ActivateSAMResultChecker : public SAMISO7816ResultChecker
{
  public:
    ActivateSAMResultChecker()
    {
        AddCheck(0x69, 0x85, "SAM is already activated",
                 CardException::CONDITIONS_OF_USE_NOT_SATISFIED);
    }
};

// RAII class to temporarily push a resultchecker to a command adapter.
class ResultCheckerGuard
{
  public:
    ResultCheckerGuard(std::shared_ptr<ReaderCardAdapter> rca,
                       std::shared_ptr<ResultChecker> rc)
        : rca_(rca)
    {
        oldrc_ = rca->getResultChecker();
        rca->setResultChecker(rc);
    }

    ~ResultCheckerGuard()
    {
        rca_->setResultChecker(oldrc_);
    }

  private:
    std::shared_ptr<ReaderCardAdapter> rca_;
    std::shared_ptr<ResultChecker> oldrc_;
};

void SAMAV3ISO7816Commands::ext_ActivateSAM()
{
    ResultCheckerGuard rcg(getReaderCardAdapter(),
                           std::make_shared<ActivateSAMResultChecker>());
    auto desfire_key = std::make_shared<DESFireKey>();
    desfire_key->setKeyType(DESFireKeyType::DF_KEY_AES);
    lockUnlock(desfire_key, SAMLockUnlock::ActivateSAMAv3, 0, 0, 0);
}

}
