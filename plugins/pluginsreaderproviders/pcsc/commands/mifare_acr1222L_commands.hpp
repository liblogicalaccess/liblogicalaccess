//
// Created by xaqq on 7/30/15.
//

#ifndef LIBLOGICALACCESS_MIFARE_ACR1222L_COMMANDS_HPP
#define LIBLOGICALACCESS_MIFARE_ACR1222L_COMMANDS_HPP

#include "mifarepcsccommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFAREACR1222L "MifareACR1222"

class LIBLOGICALACCESS_API MifareACR1222LCommands : public MifarePCSCCommands
{
  public:
    MifareACR1222LCommands()
        : MifarePCSCCommands(CMD_MIFAREACR1222L)
    {
    }

    explicit MifareACR1222LCommands(std::string ct)
        : MifarePCSCCommands(ct)
    {
    }

    virtual ~MifareACR1222LCommands() = default;
    void increment(uint8_t blockno, uint32_t value) override;

    void decrement(uint8_t blockno, uint32_t value) override;
};
}

#endif // LIBLOGICALACCESS_MIFARE_ACR1222L_COMMANDS_HPP
