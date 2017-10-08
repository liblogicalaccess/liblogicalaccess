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
    class EncapsulateGuard
    {
    public:
        EncapsulateGuard(Commands *cmd, bool level_1);
        ~EncapsulateGuard();

    private:
        class Adapter : public ReaderCardAdapter
        {
        public:
            virtual std::vector<unsigned char>
                    adaptCommand(const std::vector<unsigned char> &command) override;

            virtual std::vector<unsigned char>
                    adaptAnswer(const std::vector<unsigned char> &answer) override;

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
        virtual bool AESAuthenticateSL1(std::shared_ptr<AES128Key> key) override
        {
            EncapsulateGuard eg(this, true);
            return MifarePlusSL1Commands::AESAuthenticateSL1(key);
        }
    };

    class MifarePlusSpringcardSL1Commands
        : public MifarePlusSL1PCSCCommands
         //public MifareSpringCardCommands
    {
      public:
        virtual bool AESAuthenticate(std::shared_ptr<AES128Key> ptr,
                                     uint16_t keyslot) override;

		virtual bool AESAuthenticateSL1(std::shared_ptr<AES128Key> ptr) override;

		virtual bool switchLevel3(std::shared_ptr<AES128Key> key) override;

    protected:
        virtual void authenticate(unsigned char blockno,
                                  unsigned  char keyno,
                                  MifareKeyType keytype) override;

        MifareSpringCardCommands mscc_;

    private:
    };
}
