/**
 * \file mifareultralightcacsacrcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - ACS ACR.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP

#include "mifareultralightcpcsccommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCACSACR "MifareUltralightCACSACR"
/**
 * \brief The Mifare Ultralight C commands class for ACS ACR reader.
 */
class LIBLOGICALACCESS_API MifareUltralightCACSACRCommands
    : public MifareUltralightCPCSCCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareUltralightCACSACRCommands();

    explicit MifareUltralightCACSACRCommands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifareUltralightCACSACRCommands();

  protected:
    ByteVector sendGenericCommand(const ByteVector &data) override;
};
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP */