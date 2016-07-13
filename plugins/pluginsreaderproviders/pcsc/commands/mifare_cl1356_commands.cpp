#include "mifare_cl1356_commands.hpp"

namespace logicalaccess
{
    void MifareCL1356Commands::increment(uint8_t blockno, uint32_t value)
    {
        std::vector<unsigned char> buf;

        buf.push_back(0); // MSB block no
        buf.push_back(blockno);
        // Documentation states that the reader expects a signed 4byte number.
        int32_t num = static_cast<int32_t>(value);

        buf.push_back(static_cast<unsigned char>(num & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));

        buf.push_back(0); // two bytes destination address.
        buf.push_back(blockno);

        uint8_t p2 = 1; // perform TRANSFER after operation.
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9F, 0x21, p2, 0x08, buf);
    }

    void MifareCL1356Commands::decrement(uint8_t blockno, uint32_t value)
    {
        std::vector<unsigned char> buf;

        buf.push_back(0); // MSB block no
        buf.push_back(blockno);
        // Documentation states that the reader expects a signed 4byte number.
        int32_t num = static_cast<int32_t>(value);

        buf.push_back(static_cast<unsigned char>(num & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 8) & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 16) & 0xff));
        buf.push_back(static_cast<unsigned char>((num >> 24) & 0xff));

        buf.push_back(0); // two bytes destination address.
        buf.push_back(blockno);

        uint8_t p2 = 1; // perform TRANSFER after operation.
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9F, 0x22, p2, 0x08, buf);
    }
}
