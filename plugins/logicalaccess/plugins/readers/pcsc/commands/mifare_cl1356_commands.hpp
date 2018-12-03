#ifndef LIBLOGICALACCESS_MIFARE_CL1356_COMMANDS_HPP
#define LIBLOGICALACCESS_MIFARE_CL1356_COMMANDS_HPP

#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFARECL1356 "MifareCL1356"

class LLA_READERS_PCSC_API MifareCL1356Commands : public MifarePCSCCommands
{
  public:
    MifareCL1356Commands()
        : MifarePCSCCommands(CMD_MIFARECL1356)
    {
    }

    explicit MifareCL1356Commands(std::string ct)
        : MifarePCSCCommands(ct)
    {
    }
    virtual ~MifareCL1356Commands() = default;

    void increment(uint8_t blockno, uint32_t value) override;

    void decrement(uint8_t blockno, uint32_t value) override;
};
}

#endif // LIBLOGICALACCESS_MIFARE_ACR1222L_COMMANDS_HPP
