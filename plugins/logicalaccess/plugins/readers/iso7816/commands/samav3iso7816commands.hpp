/**
 * \file SAMAV3ISO7816Commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3ISO7816Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV3ISO7816CARDPROVIDER_HPP
#define LOGICALACCESS_SAMAV3ISO7816CARDPROVIDER_HPP

#include <logicalaccess/plugins/cards/samav/samcommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samav2commands.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_SAMAV3ISO7816 "SAMAV3ISO7816"

/**
 * \brief The SAM AV3 base commands class.
 */
class LLA_READERS_ISO7816_API SAMAV3ISO7816Commands
    : public SAMAV2ISO7816Commands<KeyEntryAV2Information, SETAV2>
#ifndef SWIG
      ,
      public SAMAV3Commands<KeyEntryAV2Information, SETAV2>
#endif
{
  public:
    /**
     * \brief Constructor.
     */
    SAMAV3ISO7816Commands();

    explicit SAMAV3ISO7816Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~SAMAV3ISO7816Commands();
    
    ByteVector encipherKeyEntry(unsigned char keyno,
                          unsigned char targetKeyno,
                          unsigned short changeCounter,
                          unsigned char channel = 0,
                          const ByteVector& targetSamUid = ByteVector(),
                          const ByteVector& divInput = ByteVector()) override;

    std::shared_ptr<Chip> getChip() const override
    {
        return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getChip();
    }

    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return SAMISO7816Commands<KeyEntryAV2Information, SETAV2>::getReaderCardAdapter();
    }
};
}

#endif /* LOGICALACCESS_SAMAV3ISO7816COMMANDS_HPP */