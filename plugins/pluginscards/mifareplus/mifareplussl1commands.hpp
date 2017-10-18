/**
 * \file mifareplussl1commands.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP

#include <logicalaccess/cards/aes128key.hpp>
#include "../mifare/mifarecommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREPLUSSL1 	"MifarePlusSL1"

    class MifarePlusChip;

    /**
     * The API offered to talk to a Mifare Plus SL1 card.
     * It extends the interface of Mifare card and provide
     * a few call that implement features from the SL1.
     *
     * This class implements all method in order to compile, but call
     * for Mifare classic shall not be dispatched to this class;
     */
    class LIBLOGICALACCESS_API MifarePlusSL1Commands : public MifareCommands
    {
    public:
		MifarePlusSL1Commands() : MifareCommands(CMD_MIFAREPLUSSL1) {}

        explicit MifarePlusSL1Commands(std::string ct) : MifareCommands(ct) {}

        /**
         * Perform AES Key authentication against the card.
         *
         * @note The keyslot allows us to reuse this code for SL1 AES authentication
         * and security-level switching (to level2).
         *
         * @param key The key to use
         * @param keyslot where the key is located on the card.
         */
        virtual bool AESAuthenticate(std::shared_ptr<AES128Key> key, uint16_t keyslot);

        virtual bool AESAuthenticateSL1(std::shared_ptr<AES128Key> key);

        /**
         * Switch the card to Security Level 2 using the key specificied
         * as a parameter
         */
        virtual bool switchLevel2(std::shared_ptr<AES128Key> key);

		/**
		 * Move the card to level 3
		 */
		virtual bool switchLevel3(std::shared_ptr<AES128Key> key);


	    ByteVector readBinary(unsigned char blockno, size_t len) override;

	    void updateBinary(unsigned char blockno,
                                  const ByteVector &buf) override;

	    bool loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol) override;

	    void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype, std::shared_ptr<MifareKey> key) override;

	    void authenticate(unsigned char blockno, unsigned char keyno,
                                  MifareKeyType keytype) override;

	    void authenticate(unsigned char blockno,
                                  std::shared_ptr<KeyStorage> key_storage,
                                  MifareKeyType keytype) override;

	    void increment(uint8_t blockno, uint32_t value) override;

	    void decrement(uint8_t blockno, uint32_t value) override;
    };

    /**
    * `MifareSL1Impl` is a class that implements
    * the additional SL1 commands.
    * `MifareClassicImpl` is a class that provide support
    * for the MifareClassic command set.
    *
    * The MifarePlusSL1Policy is templated with the concrete
    * implementation class for commands.
    */
    template<typename MifareSL1Impl,
            typename MifareClassicImpl>
    class MifarePlusSL1Policy: public MifarePlusSL1Commands
    {
    public:
		MifarePlusSL1Policy() {}

        void fixup()
        {
            classic_impl.setChip(getChip());
            classic_impl.setReaderCardAdapter(getReaderCardAdapter());

            sl1_impl_.setChip(getChip());
            sl1_impl_.setReaderCardAdapter(getReaderCardAdapter());
        }

	    bool AESAuthenticateSL1(std::shared_ptr<AES128Key> key) override
        {
            fixup();
            return sl1_impl_.AESAuthenticateSL1(key);
        }

	    void authenticate(unsigned char blockno,
                                  unsigned char keyno,
                                  MifareKeyType keytype) override
        {
            fixup();
            classic_impl.authenticate(blockno, keyno, keytype);
        }

	    ByteVector readBinary(unsigned char blockno, size_t len)override
        {
            fixup();
            return classic_impl.readBinary(blockno, len);
        }

	    void updateBinary(unsigned char blockno,
                                  const ByteVector &buf)override
        {
            fixup();
            classic_impl.updateBinary(blockno, buf);
        }

	    bool loadKey(unsigned char keyno, MifareKeyType keytype, std::shared_ptr<MifareKey> key, bool vol)override
        {
            fixup();
            return classic_impl.loadKey(keyno, keytype, key, vol);
        }

	    void loadKey(std::shared_ptr<Location> location, MifareKeyType keytype, std::shared_ptr<MifareKey> key)override
        {
            fixup();
			classic_impl.loadKey(location, keytype, key);
        }

	    void authenticate(unsigned char blockno,
                                  std::shared_ptr<KeyStorage> key_storage,
                                  MifareKeyType keytype) override
        {
            fixup();
            classic_impl.authenticate(blockno, key_storage, keytype);
        }

	    void increment(uint8_t blockno, uint32_t value)override
        {
            fixup();
            classic_impl.increment(blockno, value);
        }

	    void decrement(uint8_t blockno, uint32_t value) override
        {
            fixup();
            classic_impl.decrement(blockno, value);
        }

        MifareSL1Impl sl1_impl_;
        MifareClassicImpl classic_impl;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSL1COMMANDS_HPP */