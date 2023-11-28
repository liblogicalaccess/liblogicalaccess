/**
 * \file iso15693pcsccommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO15693 PC/SC Commands
 */

#ifndef LOGICALACCESS_ISO15693PCSCCOMMANDS_HPP
#define LOGICALACCESS_ISO15693PCSCCOMMANDS_HPP

#include <logicalaccess/plugins/cards/iso15693/iso15693commands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_ISO15693PCSC "ISO15693PCSC"

/**
 * \brief A ISO 15693 PC/SC commands base class.
 */
class LLA_READERS_PCSC_API ISO15693PCSCCommands : public ISO15693Commands
{
  public:
    /**
     * \brief Constructor.
     */
    ISO15693PCSCCommands();

    explicit ISO15693PCSCCommands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~ISO15693PCSCCommands();

    void stayQuiet() override;
    ByteVector readBlock(size_t block, size_t le = 0) override;
    void writeBlock(size_t block, const ByteVector &data) override;
    void lockBlock(size_t block) override;
    void writeAFI(size_t afi) override;
    void lockAFI() override;
    void writeDSFID(size_t dsfid) override;
    void lockDSFID() override;
    SystemInformation getSystemInformation() override;
    unsigned char getSecurityStatus(size_t block) override;

    /**
     * \brief Get the PC/SC reader/card adapter.
     * \return The PC/SC reader/card adapter.
     */
    virtual std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter()
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter());
    }
};
}

#endif /* LOGICALACCESS_ISO15693PCSCCOMMANDS_HPP */