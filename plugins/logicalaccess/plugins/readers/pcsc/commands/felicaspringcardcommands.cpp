/**
* \file felicaspringcardcommands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief FeliCa SpringCard commands.
*/

#include <logicalaccess/plugins/readers/pcsc/commands/felicaspringcardcommands.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>

namespace logicalaccess
{
FeliCaSpringCardCommands::FeliCaSpringCardCommands()
    : FeliCaCommands(CMD_FELICASPRINGCARD)
{
}

FeliCaSpringCardCommands::FeliCaSpringCardCommands(std::string ct)
    : FeliCaCommands(ct)
{
}

FeliCaSpringCardCommands::~FeliCaSpringCardCommands()
{
}

std::vector<unsigned short> FeliCaSpringCardCommands::getSystemCodes()
{
    return std::vector<unsigned short>();
}

std::vector<unsigned short>
FeliCaSpringCardCommands::requestServices(const std::vector<unsigned short> &)
{
    return std::vector<unsigned short>();
}

unsigned char FeliCaSpringCardCommands::requestResponse()
{
    return 0x00;
}

ByteVector FeliCaSpringCardCommands::read(const std::vector<unsigned short> &codes,
                                          const std::vector<unsigned short> &blocks)
{
    ByteVector data;
    for (unsigned int i = 0; i < codes.size(); ++i)
    {
        ByteVector cmd;
        // Set FeliCa Service Code to use
        cmd.push_back(static_cast<unsigned char>((codes[i] >> 8) & 0xff));
        cmd.push_back(static_cast<unsigned char>(codes[i] & 0xff));
        getPCSCReaderCardAdapter()->sendAPDUCommand(
            0xFF, 0xFB, 0xFC, 0x01, static_cast<unsigned char>(cmd.size()), cmd);

        for (unsigned int b = 0; b < blocks.size(); ++b)
        {
            ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
                0xFF, 0xB0, 0x00, static_cast<unsigned char>(blocks[b] & 0xff), 16);
            data.insert(data.end(), result.begin(), result.end() - 2);
        }
    }
    return data;
}

void FeliCaSpringCardCommands::write(const std::vector<unsigned short> &codes,
                                     const std::vector<unsigned short> &blocks,
                                     const ByteVector &data)
{
    unsigned int offset = 0;
    for (unsigned int i = 0; i < codes.size(); ++i)
    {
        ByteVector cmd;
        // Set FeliCa Service Code to use
        cmd.push_back(static_cast<unsigned char>((codes[i] >> 8) & 0xff));
        cmd.push_back(static_cast<unsigned char>(codes[i] & 0xff));
        getPCSCReaderCardAdapter()->sendAPDUCommand(
            0xFF, 0xFB, 0xFC, 0x02, static_cast<unsigned char>(cmd.size()), cmd);

        for (unsigned int b = 0; b < blocks.size(); ++b)
        {
            if (offset >= data.size())
                break;

            unsigned char len = static_cast<unsigned char>(
                (offset + 16 <= data.size()) ? 16 : data.size() - offset);
            cmd = ByteVector(data.begin() + offset, data.begin() + offset + len);
            if (len != 16)
            {
                cmd.resize(16, 0x00);
            }
            getPCSCReaderCardAdapter()->sendAPDUCommand(
                0xFF, 0xD6, 0x00, static_cast<unsigned char>(blocks[b] & 0xff), len, cmd);
            offset += 16;
        }
    }
}
}