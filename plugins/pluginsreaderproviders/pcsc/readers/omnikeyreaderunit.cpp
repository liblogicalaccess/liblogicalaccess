/**
 * \file omnikeyreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey reader unit.
 */

#include "../readers/omnikeyreaderunit.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    std::map<std::string, bool> OmnikeyReaderUnit::d_isSecureConnectionMode;

    OmnikeyReaderUnit::OmnikeyReaderUnit(const std::string& name)
        : PCSCReaderUnit(name)
    {
    }

    OmnikeyReaderUnit::~OmnikeyReaderUnit()
    {
    }

    std::string OmnikeyReaderUnit::getReaderSerialNumber()
    {
        std::string ret;

        std::vector<unsigned char> ucReceivedData;

        //This Command is from ICAO Command Set (Test-Commands). If you use ICAO Test-Commands then the driver stop the tracking (the reader is � stuck � ).
        ucReceivedData = getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9A, 0x01, 0x05, 0x00);
        //After using this command you must "Close" the Test-Mode with the following command:
        size_t le = ucReceivedData.size() - 2;

        if (le > 0)
        {
            ret = BufferHelper::getStdString(ucReceivedData);
        }

        try
        {
            getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x9A, 0x04, 0x01);
        }
        catch (std::exception&)
        {
        }

        return ret;
    }

    bool OmnikeyReaderUnit::getIsSecureConnectionMode()
    {
        if (d_isSecureConnectionMode.find(getConnectedName()) == d_isSecureConnectionMode.end())
        {
            return false;
        }

        return d_isSecureConnectionMode[getConnectedName()];
    }

    void OmnikeyReaderUnit::setIsSecureConnectionMode(bool isSecure)
    {
        d_isSecureConnectionMode[getConnectedName()] = isSecure;
    }

    bool OmnikeyReaderUnit::waitRemoval(unsigned int maxwait)
    {
        /*if (getIsSecureConnectionMode())
        {
        SecureModeCloseSession(getHandle());
        setIsSecureConnectionMode(false);
        }*/

        return PCSCReaderUnit::waitRemoval(maxwait);
    }

    bool OmnikeyReaderUnit::waitInsertion(unsigned int maxwait)
    {
        return PCSCReaderUnit::waitInsertion(maxwait);
    }
}