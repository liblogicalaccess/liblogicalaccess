/**
 * \file mifareultralightccommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare Ultralight C commands.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCCOMMANDS_HPP

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcommands.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcaccessinfo.hpp>

namespace logicalaccess
{
class MifareUltralightCChip;

/**
 * \brief The Mifare Ultralight C commands class.
 */
class LLA_CARDS_MIFAREULTRALIGHT_API MifareUltralightCCommands
{
  public:
    virtual ~MifareUltralightCCommands() = default;

    /**
 * \brief Authenticate to the chip.
 * \param aiToUse The access information to use for authentication.
 */
    void authenticate(std::shared_ptr<AccessInfo> aiToUse);

    /**
     * \brief Change the chip authentication key.
     * \param aiToWrite The access information to write.
     */
    void changeKey(std::shared_ptr<AccessInfo> aiToWrite);

    /**
     * \brief Change the chip authentication key.
     * \param key The new key.
     */
    virtual void changeKey(std::shared_ptr<TripleDESKey> key);

    /**
     * \brief Set a page as read-only.
     * \param page The page to lock.
     */
    virtual void lockPage(int page);

    /**
     * \brief Authenticate to the chip.
     * \param authkey The authentication key.
     */
    virtual void authenticate(std::shared_ptr<TripleDESKey> authkey) = 0;

    virtual void writePage(int page, const ByteVector &buf) = 0;

  protected:
    std::shared_ptr<MifareUltralightCChip> getMifareUltralightCChip();

    virtual std::shared_ptr<MifareUltralightChip> getMifareUltralightChip() = 0;
};
}

#endif