/**
 * \file SAMAV2ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV2ISO7816CARDPROVIDER_HPP

#include "samav2/samcommands.hpp"
#include "samav1iso7816commands.hpp"
#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samav2/samcrypto.hpp"
#include "samav2/samkeyentry.hpp"
#include "samav2/samcrypto.hpp"
#include "samav2/samav2commands.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define AV2_HEADER_LENGTH 0x05
#define AV2_HEADER_LENGTH_WITH_LE 0x06
#define AV2_LC_POS 0x04
#define CMD_SAMAV2ISO7816 "SAMAV2ISO7816"

    /**
     * \brief The DESFire base commands class.
     */
    class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMISO7816Commands<KeyEntryAV2Information, SETAV2>
    , public SAMAV2Commands < KeyEntryAV2Information, SETAV2 >
    {
    public:

        /**
         * \brief Constructor.
         */
        SAMAV2ISO7816Commands();

        explicit SAMAV2ISO7816Commands(std::string);

        /**
         * \brief Destructor.
         */
        virtual ~SAMAV2ISO7816Commands();

	    void authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno) override;

	    std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > getKeyEntry(unsigned char keyno) override;
	    std::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno) override;

	    void changeKUCEntry(unsigned char kucno, std::shared_ptr<SAMKucEntry> kucentry, std::shared_ptr<DESFireKey> key) override;
	    void changeKeyEntry(unsigned char keyno, std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, std::shared_ptr<DESFireKey> key) override;

	    ByteVector transmit(ByteVector cmd, bool first = true, bool last = true) override;

	    ByteVector dumpSecretKey(unsigned char keyno, unsigned char keyversion, ByteVector divInpu) override;

	    void activateOfflineKey(unsigned char keyno, unsigned char keyversion, ByteVector divInpu) override;

	    ByteVector decipherOfflineData(ByteVector data) override;

	    ByteVector encipherOfflineData(ByteVector data) override;

		virtual ByteVector cmacOffline(const ByteVector& data);

		std::shared_ptr<Chip> getChip() const override { return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getChip(); }

		std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override { return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getReaderCardAdapter(); }

    protected:

        void generateSessionKey(ByteVector rnd1, ByteVector rnd2);

        ByteVector createfullProtectionCmd(ByteVector cmd);

        ByteVector verifyAndDecryptResponse(ByteVector response);

        static void getLcLe(ByteVector cmd, bool& lc, unsigned char& lcvalue, bool& le);

        ByteVector generateEncIV(bool encrypt) const;

        ByteVector d_macSessionKey;

        ByteVector d_lastMacIV;

        unsigned int d_cmdCtr;
    };
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */