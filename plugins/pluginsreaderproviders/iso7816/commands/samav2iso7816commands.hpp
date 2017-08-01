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
    class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMAV2Commands<KeyEntryAV2Information, SETAV2>
    {
    public:

        /**
         * \brief Constructor.
         */
        SAMAV2ISO7816Commands();

		SAMAV2ISO7816Commands(std::string);

        /**
         * \brief Destructor.
         */
        virtual ~SAMAV2ISO7816Commands();

        virtual void authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno);

        virtual std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > getKeyEntry(unsigned char keyno);
        virtual std::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno);

        virtual void changeKUCEntry(unsigned char kucno, std::shared_ptr<SAMKucEntry> kucentry, std::shared_ptr<DESFireKey> key);
        virtual void changeKeyEntry(unsigned char keyno, std::shared_ptr<SAMKeyEntry<KeyEntryAV2Information, SETAV2> > keyentry, std::shared_ptr<DESFireKey> key);

        virtual std::vector<unsigned char> transmit(std::vector<unsigned char> cmd, bool first = true, bool last = true);

        virtual std::vector<unsigned char> dumpSecretKey(unsigned char keyno, unsigned char keyversion, std::vector<unsigned char> divInpu);

		// SAMISO7816Commands<KeyEntryAV2Information, SETAV2>

		std::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() { return bridgeISO->getISO7816ReaderCardAdapter(); };

		//virtual std::vector<unsigned char> transmit(std::vector<unsigned char> cmd, bool first = true, bool last = true) { bridgeISO->transmit(cmd, first, last); }

		virtual SAMVersion getVersion() { return bridgeISO->getVersion(); }

		virtual std::vector<unsigned char> decipherData(std::vector<unsigned char> data, bool islastdata) { return bridgeISO->decipherData(data, islastdata); }

		virtual std::vector<unsigned char> encipherData(std::vector<unsigned char> data, bool islastdata) { return bridgeISO->encipherData(data, islastdata); }

		virtual void disableKeyEntry(unsigned char keyno) { bridgeISO->disableKeyEntry(keyno); }

		virtual std::vector<unsigned char> dumpSessionKey() { return bridgeISO->dumpSessionKey(); }

		virtual std::string getSAMTypeFromSAM() { return bridgeISO->getSAMTypeFromSAM(); }

		virtual std::shared_ptr<SAMDESfireCrypto> getCrypto() { return bridgeISO->getCrypto(); };
		virtual void setCrypto(std::shared_ptr<SAMDESfireCrypto> t) { bridgeISO->setCrypto(t); };

		virtual void lockUnlock(std::shared_ptr<DESFireKey> masterKey, SAMLockUnlock state, unsigned char keyno, unsigned char unlockkeyno, unsigned char unlockkeyversion) { bridgeISO->lockUnlock(masterKey, state, keyno, unlockkeyno, unlockkeyversion); }

		void selectApplication(std::vector<unsigned char> aid) { bridgeISO->selectApplication(aid); }

		std::vector<unsigned char> changeKeyPICC(const ChangeKeyInfo& info, const ChangeKeyDiversification& diversifycation) { return bridgeISO->changeKeyPICC(info, diversifycation); }

		void truncateMacBuffer(std::vector<unsigned char>& data) { bridgeISO->truncateMacBuffer(data); }

		void generateAuthEncKey(std::vector<unsigned char> keycipher, std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2) { bridgeISO->generateAuthEncKey(keycipher, rnd1, rnd2); }

		std::shared_ptr<SAMISO7816Commands<KeyEntryAV2Information, SETAV2> > getSamIsoCmd() { return this->bridgeISO; }

    protected:

        void generateSessionKey(std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2);

        std::vector<unsigned char> createfullProtectionCmd(std::vector<unsigned char> cmd);

        std::vector<unsigned char> verifyAndDecryptResponse(std::vector<unsigned char> response);

        void getLcLe(std::vector<unsigned char> cmd, bool& lc, unsigned char& lcvalue, bool& le);

        std::vector<unsigned char> generateEncIV(bool encrypt);

        std::vector<unsigned char> d_macSessionKey;

        std::vector<unsigned char> d_lastMacIV;

        unsigned int d_cmdCtr;

		std::shared_ptr<SAMDESfireCrypto>& getCryptoRef() { return bridgeISO->getCryptoRef(); }

		unsigned char& getClaRef() { return bridgeISO->getClaRef(); }

		std::vector<unsigned char>& getAuthKeyRef() { return bridgeISO->getAuthKeyRef(); }

		std::vector<unsigned char>& getSessionKeyRef() { return bridgeISO->getSessionKeyRef(); }

		std::vector<unsigned char>& getLastSessionIVRef() { return bridgeISO->getLastSessionIVRef(); }

	private:
			
		std::shared_ptr<SAMISO7816Commands<KeyEntryAV2Information, SETAV2> > bridgeISO;
    };
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */