/**
 * \file mifareultralightcacscommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - SpringCard.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP

#include <logicalaccess/plugins/readers/pcsc/commands/mifareultralightcpcsccommands.hpp>

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCSPRINGCARD "MifareUltralightCSpringCard"

/**
 * \brief The Mifare Ultralight C commands class for SpringCard reader.
 */
class LIBLOGICALACCESS_API MifareUltralightCSpringCardCommands
    : public MifareUltralightCPCSCCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareUltralightCSpringCardCommands();

    explicit MifareUltralightCSpringCardCommands(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~MifareUltralightCSpringCardCommands();

  protected:
    void startGenericSession() override;

    void stopGenericSession() override;

    ByteVector sendGenericCommand(const ByteVector &data) override;
};
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP */