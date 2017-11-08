//
// Created by xaqq on 6/22/15.
//

#include <logicalaccess/plugins/readers/pcsc/commands/mifareomnikeyxx21commands.hpp>

namespace logicalaccess
{
MifareOmnikeyXX21Commands::MifareOmnikeyXX21Commands()
    : MifarePCSCCommands(CMD_MIFAREOMNIKEYXX21)
{
}

MifareOmnikeyXX21Commands::MifareOmnikeyXX21Commands(std::string ct)
    : MifarePCSCCommands(ct)
{
}

MifareOmnikeyXX21Commands::~MifareOmnikeyXX21Commands()
{
}

void MifareOmnikeyXX21Commands::increment(uint8_t blockno, uint32_t value)
{
    ByteVector buf(4);
    memcpy(&buf[0], &value, 4);
    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD4, 0x00, blockno, 4, buf);
}

void MifareOmnikeyXX21Commands::decrement(uint8_t blockno, uint32_t value)
{
    ByteVector buf(4);
    memcpy(&buf[0], &value, 4);
    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD8, 0x00, blockno, 4, buf);
}
}
