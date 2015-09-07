//
// Created by xaqq on 6/22/15.
//

#include "mifareomnikeyxx21commands.hpp"

namespace logicalaccess
{
    MifareOmnikeyXX21Commands::MifareOmnikeyXX21Commands()
            : MifarePCSCCommands()
    {
    }

    MifareOmnikeyXX21Commands::~MifareOmnikeyXX21Commands()
    {
    }

    void MifareOmnikeyXX21Commands::increment(uint8_t blockno, uint32_t value)
    {
        std::vector<uint8_t> buf(4);
        memcpy(&buf[0], &value, 4);
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD4, 0x00, blockno,
                                                    4, buf);
    }

    void MifareOmnikeyXX21Commands::decrement(uint8_t blockno, uint32_t value)
    {
        std::vector<uint8_t> buf(4);
        memcpy(&buf[0], &value, 4);
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD8, 0x00, blockno,
                                                    4, buf);
    }
}
