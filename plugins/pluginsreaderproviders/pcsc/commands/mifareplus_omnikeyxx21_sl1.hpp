//
// Created by xaqq on 7/1/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUS_OMNIKEYXX21_SL1_H
#define LIBLOGICALACCESS_MIFAREPLUS_OMNIKEYXX21_SL1_H

#include "mifareomnikeyxx21commands.hpp"
#include "mifareplus_pcsc_sl1.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API MifarePlusOmnikeyXX21SL1Commands
    : public MifarePlusSL1PCSCCommands
{
  public:
    /**
     * The implementation for XX21 needs to use the GENERIC SESSION
     * provided by HID in order to send the AES Authenticate to the card.
     *
     * This doesn't work under Linux, presumably because the driver doesn't
     * support it.
     */
    bool AESAuthenticate(std::shared_ptr<AES128Key> ptr, uint16_t keyslot) override;

  private:
    /**
     * This class setup a Generic Session channel to send
     * mifare plus commands directly to the card.
     *
     * The channel is setup in the constructor, and tore down
     * in the destructor.
     *
     * The ReaderCardAdapter from the command object is swapped out
     * by this one (and the old one is set when this object goes
     * out of scope)
     */
    class GenericSessionGuard
    {
      public:
        explicit GenericSessionGuard(Commands *cmd);
        ~GenericSessionGuard();

      private:
        class Adapter : public ReaderCardAdapter
        {
            ByteVector adaptCommand(const ByteVector &command) override;

            ByteVector adaptAnswer(const ByteVector &answer) override;
        };

        /**
         * The "old" rca object.
         */
        std::shared_ptr<PCSCReaderCardAdapter> rca_;

        Commands *cmd_;

        std::shared_ptr<Adapter> adapter_;
    };
};
}

#endif // LIBLOGICALACCESS_MIFAREPLUS_OMNIKEYXX21_SL1_H
