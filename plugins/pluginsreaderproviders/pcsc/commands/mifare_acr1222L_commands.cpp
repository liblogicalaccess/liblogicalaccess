//
// Created by xaqq on 7/30/15.
//

#include "mifare_acr1222L_commands.hpp"

void logicalaccess::MifareACR1222LCommands::increment(uint8_t blockno, uint32_t value)
{
    std::vector<unsigned char> buf;
    buf.push_back(0x01);
    // Documentation states that the reader expects a signed 4byte number.
    int32_t num = static_cast<int32_t>(value);

    buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>(num & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD7, 0x00, blockno, 0x05, buf);
}

void logicalaccess::MifareACR1222LCommands::decrement(uint8_t blockno, uint32_t value)
{
    std::vector<unsigned char> buf;
    buf.push_back(0x02);
    // Documentation states that the reader expects a signed 4byte number.
    int32_t num = static_cast<int32_t>(value);

    buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
    buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
    buf.push_back(static_cast<unsigned char>(num & 0xff));

    getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xD7, 0x00, blockno, 0x05, buf);
}
