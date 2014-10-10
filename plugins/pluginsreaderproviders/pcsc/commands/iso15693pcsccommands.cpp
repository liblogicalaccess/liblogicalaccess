/**
 * \file iso15693pcsccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO 15693 commands for PC/SC readers.
 */

#include "../commands/iso15693pcsccommands.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "iso15693chip.hpp"

namespace logicalaccess
{
    ISO15693PCSCCommands::ISO15693PCSCCommands()
        : ISO15693Commands()
    {
    }

    ISO15693PCSCCommands::~ISO15693PCSCCommands()
    {
    }

    void ISO15693PCSCCommands::stayQuiet()
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(0x01);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 3, command);
    }

    bool ISO15693PCSCCommands::readBlock(size_t block, void* data, size_t datalen, size_t le)
    {
        bool ret = false;
        std::vector<unsigned char> result;
        unsigned char p1 = (block & 0xffff) >> 8;
        unsigned char p2 = static_cast<unsigned char>(block & 0xff);

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0xb0, p1, p2, static_cast<unsigned char>(le));

        if (datalen >= result.size() - 2)
        {
            memcpy(data, &result[0], result.size() - 2);
            ret = true;
        }

        return ret;
    }

    bool ISO15693PCSCCommands::writeBlock(size_t block, const void* data, size_t datalen)
    {
        bool ret = false;
        std::vector<unsigned char> result;
        unsigned char p1 = (block & 0xffff) >> 8;
        unsigned char p2 = static_cast<unsigned char>(block & 0xff);

        std::vector<unsigned char> vector_data((unsigned char*)data, (unsigned char*)data + datalen);
        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0xd6, p1, p2, static_cast<unsigned char>(datalen), vector_data);

        if (datalen >= result.size() - 2)
        {
            ret = true;
        }

        return ret;
    }

    void ISO15693PCSCCommands::lockBlock(size_t block)
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(0x00);
        command.push_back((block & 0xffff) >> 8);
        command.push_back(static_cast<unsigned char>(block & 0xff));

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 5, command);
    }

    void ISO15693PCSCCommands::writeAFI(size_t afi)
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x02);
        command.push_back(0x00);
        command.push_back((unsigned char)afi);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 4, command);
    }

    void ISO15693PCSCCommands::lockAFI()
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x02);
        command.push_back(0x00);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 3, command);
    }

    void ISO15693PCSCCommands::writeDSFID(size_t dsfid)
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x03);
        command.push_back(0x00);
        command.push_back((unsigned char)dsfid);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x01, 4, command);
    }

    void ISO15693PCSCCommands::lockDSFID()
    {
        std::vector<unsigned char> command;
        command.push_back(0x01);
        command.push_back(0x03);
        command.push_back(0x00);

        getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x02, 3, command);
    }

    ISO15693Commands::SystemInformation ISO15693PCSCCommands::getSystemInformation()
    {
        ISO15693Commands::SystemInformation systeminfo;
        std::vector<unsigned char> result;
        memset(&systeminfo, 0x00, sizeof(systeminfo));

        // AFI information
        try
        {
            result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x02, 0x00, 0x00);
            systeminfo.hasAFI = true;
            systeminfo.AFI = result[0];
        }
        catch (LibLogicalAccessException&)
        {
            systeminfo.hasAFI = false;
        }

        // DSFID information
        try
        {
            result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x03, 0x00, 0x00);
            systeminfo.hasDSFID = true;
            systeminfo.DSFID = result[0];
        }
        catch (LibLogicalAccessException&)
        {
            systeminfo.hasDSFID = false;
        }

        // PICC memory size information
        try
        {
            result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x04, 0x00, 0x00);
            systeminfo.hasVICCMemorySize = true;
            systeminfo.blockSize = (result[1] & 0x1f) + 1;
            systeminfo.nbBlocks = result[0] + 1;
        }
        catch (LibLogicalAccessException&)
        {
            systeminfo.hasVICCMemorySize = false;
        }

        // IC reference information
        try
        {
            result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x05, 0x00, 0x00);
            systeminfo.hasICReference = true;
            systeminfo.ICReference = result[0];
        }
        catch (LibLogicalAccessException&)
        {
            systeminfo.hasICReference = false;
        }

        return systeminfo;
    }

    unsigned char ISO15693PCSCCommands::getSecurityStatus(size_t block)
    {
        std::vector<unsigned char> result, command;
        command.push_back(0x01);
        command.push_back(0x00);
        command.push_back(0x00);
        command.push_back((block & 0xffff) >> 8);
        command.push_back(static_cast<unsigned char>(block & 0xff));

        result = getPCSCReaderCardAdapter()->sendAPDUCommand(0xff, 0x30, 0x00, 0x03, 5, command, 0x01);

        return result[0];
    }
}