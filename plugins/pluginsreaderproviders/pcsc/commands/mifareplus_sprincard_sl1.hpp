#pragma once

#include "mifareomnikeyxx21commands.hpp"
#include "mifareplus_pcsc_sl1.hpp"
#include "mifarespringcardcommands.hpp"

namespace logicalaccess
{
/**
  * This class replace the current ReaderCardAdapter with one
  * that will wrap command into a ENCAPSULATE APDU.
  *
  * The ENCAPSULATE parameters depends on what level we want to operate.
  * Example: For AES Auth in SL1, pass level_1 as true.
  *          However, if the card is SL1 and we'll perform AES auth to switch
  *          to SL3, pass level_1 as false.
  */
class LIBLOGICALACCESS_API EncapsulateGuard
{
  public:
    EncapsulateGuard(Commands *cmd, bool level_1);
    ~EncapsulateGuard();

  private:
    class Adapter : public ReaderCardAdapter
    {
      public:
        ByteVector adaptCommand(const ByteVector &command) override;

        ByteVector adaptAnswer(const ByteVector &answer) override;

        bool level_1_;
    };

    /**
     * The "old" rca object.
     */
    std::shared_ptr<PCSCReaderCardAdapter> rca_;

    Commands *cmd_;

    std::shared_ptr<Adapter> adapter_;
};


class MifarePlusSpringcardAES_SL1_Auth : public MifarePlusSL1PCSCCommands
{

  public:
    bool AESAuthenticateSL1(std::shared_ptr<AES128Key> key) override
    {
        EncapsulateGuard eg(this, true);
        return MifarePlusSL1Commands::AESAuthenticateSL1(key);
    }
};

class LIBLOGICALACCESS_API MifarePlusSpringcardSL1Commands
    : public MifarePlusSL1PCSCCommands
// public MifareSpringCardCommands
{
  public:
    bool AESAuthenticate(std::shared_ptr<AES128Key> ptr, uint16_t keyslot) override;

    bool AESAuthenticateSL1(std::shared_ptr<AES128Key> ptr) override;

    bool switchLevel3(std::shared_ptr<AES128Key> key) override;

  protected:
    void authenticate(unsigned char blockno, unsigned char keyno,
                      MifareKeyType keytype) override;

    MifareSpringCardCommands mscc_;
};
}
