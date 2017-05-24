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

    /**
     * \brief The DESFire base commands class.
     */
    class LIBLOGICALACCESS_API SAMAV2ISO7816Commands : public SAMISO7816Commands<KeyEntryAV2Information, SETAV2>
#ifndef SWIG
    , public SAMAV2Commands < KeyEntryAV2Information, SETAV2 >
#endif
    {
    public:

        /**
         * \brief Constructor.
         */
        SAMAV2ISO7816Commands();

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

    protected:

        void generateSessionKey(std::vector<unsigned char> rnd1, std::vector<unsigned char> rnd2);

        std::vector<unsigned char> createfullProtectionCmd(std::vector<unsigned char> cmd);

        std::vector<unsigned char> verifyAndDecryptResponse(std::vector<unsigned char> response);

        void getLcLe(std::vector<unsigned char> cmd, bool& lc, unsigned char& lcvalue, bool& le);

        std::vector<unsigned char> generateEncIV(bool encrypt);

        std::vector<unsigned char> d_macSessionKey;

        std::vector<unsigned char> d_lastMacIV;

        unsigned int d_cmdCtr;
    };
}

#endif /* LOGICALACCESS_SAMAV2ISO7816COMMANDS_HPP */