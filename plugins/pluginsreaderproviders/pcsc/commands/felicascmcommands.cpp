/**
* \file felicascmcommands.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief FeliCa SCM commands.
*/

#include "../commands/felicascmcommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include "../pcscreaderprovider.hpp"

namespace logicalaccess
{
FeliCaSCMCommands::FeliCaSCMCommands()
    : FeliCaCommands(CMD_FELICASCM)
{
}

FeliCaSCMCommands::FeliCaSCMCommands(std::string ct)
    : FeliCaCommands(ct)
{
}

FeliCaSCMCommands::~FeliCaSCMCommands()
{
}

std::vector<unsigned short> FeliCaSCMCommands::getSystemCodes()
{
    std::vector<unsigned short> codes;
    ByteVector result =
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x4A, 0x00, 0x00);
    // First 8 bytes = IDm
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 8, LibLogicalAccessException,
                              "Wrong system codes result.");
    unsigned char nbCodes = result[8];
    EXCEPTION_ASSERT_WITH_LOG(
        result.size() >= static_cast<unsigned int>(9 + (nbCodes * 2)),
        LibLogicalAccessException,
        "Wrong system codes result. Invalid length for excepted system codes number.");

    for (unsigned char i = 0; i < nbCodes; ++i)
    {
        unsigned short code = (result[9 + (i * 2)] << 8) + result[10 + (i * 2)];
        codes.push_back(code);
    }

    return codes;
}

std::vector<unsigned short>
FeliCaSCMCommands::requestServices(const std::vector<unsigned short> &codes)
{
    ByteVector data;
    for (unsigned int i = 0; i < codes.size(); ++i)
    {
        data.push_back(static_cast<unsigned char>((codes[i] >> 8) & 0xff));
        data.push_back(static_cast<unsigned char>(codes[i] & 0xff));
    }

    std::vector<unsigned short> versions;
    ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x42, static_cast<unsigned char>(codes.size()), 0x00, data);
    // First 8 bytes = IDm
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 8, LibLogicalAccessException,
                              "Wrong request service result.");
    unsigned char nbVersions = result[8];
    EXCEPTION_ASSERT_WITH_LOG(
        result.size() >= static_cast<unsigned int>(9 + (nbVersions * 2)),
        LibLogicalAccessException, "Wrong request service result. Invalid length for "
                                   "excepted services/areas number.");

    for (unsigned char i = 0; i < nbVersions; ++i)
    {
        unsigned short version = (result[9 + (i * 2)] << 8) + result[10 + (i * 2)];
        versions.push_back(version);
    }

    return versions;
}

unsigned char FeliCaSCMCommands::requestResponse()
{
    ByteVector result =
        getPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x44, 0x00, 0x00);
    // First 8 bytes = IDm
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 8, LibLogicalAccessException,
                              "Wrong request response result.");

    return result[8];
}

ByteVector FeliCaSCMCommands::read(const std::vector<unsigned short> &codes,
                                   const std::vector<unsigned short> &blocks)
{
    ByteVector data;
    for (unsigned int i = 0; i < codes.size(); ++i)
    {
        data.push_back(static_cast<unsigned char>((codes[i] >> 8) & 0xff));
        data.push_back(static_cast<unsigned char>(codes[i] & 0xff));
    }
    for (unsigned int i = 0; i < blocks.size(); ++i)
    {
        data.push_back(static_cast<unsigned char>((blocks[i] >> 8) & 0xff));
        data.push_back(static_cast<unsigned char>(blocks[i] & 0xff));
    }

    ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x46, static_cast<unsigned char>(codes.size()),
        static_cast<unsigned char>(blocks.size()), data);
    // First 8 bytes = IDm, then Status Flag 1 + Status Flag 2
    EXCEPTION_ASSERT_WITH_LOG(result.size() > 10, LibLogicalAccessException,
                              "Wrong read result.");
    unsigned char nbBlocks = result[10];
    EXCEPTION_ASSERT_WITH_LOG(result.size() >=
                                  static_cast<unsigned int>(11 + (nbBlocks * 16)),
                              LibLogicalAccessException,
                              "Wrong read result. Invalid length for blocks number.");

    return ByteVector(result.begin() + 11, result.begin() + 11 + (nbBlocks * 16));
}

void FeliCaSCMCommands::write(const std::vector<unsigned short> &codes,
                              const std::vector<unsigned short> &blocks,
                              const ByteVector &data)
{
    ByteVector cdata;
    for (unsigned int i = 0; i < codes.size(); ++i)
    {
        cdata.push_back(static_cast<unsigned char>((codes[i] >> 8) & 0xff));
        cdata.push_back(static_cast<unsigned char>(codes[i] & 0xff));
    }
    for (unsigned int i = 0; i < blocks.size(); ++i)
    {
        cdata.push_back(static_cast<unsigned char>((blocks[i] >> 8) & 0xff));
        cdata.push_back(static_cast<unsigned char>(blocks[i] & 0xff));
    }
    cdata.insert(cdata.end(), data.begin(), data.end());

    ByteVector result = getPCSCReaderCardAdapter()->sendAPDUCommand(
        0xFF, 0x48, static_cast<unsigned char>(codes.size()),
        static_cast<unsigned char>(blocks.size()), data);
    // First 8 bytes = IDm, then Status Flag 1 + Status Flag 2
    EXCEPTION_ASSERT_WITH_LOG(result.size() >= 10, LibLogicalAccessException,
                              "Wrong write result.");
}
}