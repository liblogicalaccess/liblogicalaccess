//
// Created by xaqq on 7/1/15.
//
#pragma once

#include "mifareomnikeyxx21commands.hpp"
#include "mifareplus_pcsc_sl1.hpp"

namespace logicalaccess
{
    class LIBLOGICALACCESS_API MifarePlusACSACR1222L_SL1Commands : public MifarePlusSL1PCSCCommands
    {
    public:
	    bool AESAuthenticate(std::shared_ptr<AES128Key> ptr,
                                     uint16_t keyslot) override;

    private:
      /**
       * This class setup a "Direct Transmit" channel to send
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
	          ByteVector
              adaptCommand(const ByteVector &command) override;

	          ByteVector
              adaptAnswer(const ByteVector &answer) override;
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
