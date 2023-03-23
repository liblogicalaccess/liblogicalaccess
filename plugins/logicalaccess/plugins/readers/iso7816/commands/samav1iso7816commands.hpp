/**
 * \file SAMAV1ISO7816Commands.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV1ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV1ISO7816CARDPROVIDER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samiso7816commands.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samcommands.hpp>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#include <string>
#include <vector>
#include <iostream>

#define DEFAULT_SAM_CLA 0x80

namespace logicalaccess
{
#define CMD_SAMAV1ISO7816 "SAMAV1ISO7816"

#ifdef SWIG
%template(SAMISO7816KeyEntrySETAV1Commands)
        SAMISO7816Commands<KeyEntryAV1Information, SETAV1>;
#endif

/**
 * \brief The SAMAV1ISO7816 commands class.
 */
class LLA_READERS_ISO7816_API SAMAV1ISO7816Commands
    : public SAMISO7816Commands<KeyEntryAV1Information, SETAV1>
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV1ISO7816Commands();

    explicit SAMAV1ISO7816Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~SAMAV1ISO7816Commands();

    std::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1>>
    getKeyEntry(unsigned char keyno) override;
    std::shared_ptr<SAMKucEntry> getKUCEntry(unsigned char kucno) override;

    void changeKUCEntry(unsigned char kucno, std::shared_ptr<SAMKucEntry> keyentry,
                        std::shared_ptr<DESFireKey> key) override;
    void
    changeKeyEntry(unsigned char keyno,
                   std::shared_ptr<SAMKeyEntry<KeyEntryAV1Information, SETAV1>> keyentry,
                   std::shared_ptr<DESFireKey> key) override;

    void authenticateHost(std::shared_ptr<DESFireKey> key, unsigned char keyno) override;
    void authenticateHost_AES_3K3DES(std::shared_ptr<DESFireKey> key,
                                     unsigned char keyno);
    void authenticateHostDES(std::shared_ptr<DESFireKey> key, unsigned char keyno);
};
}

#endif /* LOGICALACCESS_SAMAV1ISO7816COMMANDS_HPP */