#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/omnikeylanxx21readerunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/omnikeyxx22readerunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/omnikeyxx25readerunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/scmreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/springcardreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/cherryreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacrreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacr1222lreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/id3readerunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/readers/pcsc/lla_readers_pcsc_api.hpp>

extern "C" {

/**
* Attempt to create a ReaderUnit object for a reader with name readName.
* This function returns NULL if there is no match, is does NOT create a default
* PCSCReaderUnit.
*/
LLA_READERS_PCSC_API void getReaderUnit(const std::string &readerName,
                                        std::shared_ptr<logicalaccess::ReaderUnit> &u)
{
    if (readerName.find("OMNIKEY") != std::string::npos)
    {
        if (readerName.find("x21") != std::string::npos ||
            readerName.find("5321") != std::string::npos ||
            readerName.find("6321") != std::string::npos)
        {
            if (readerName.find("LAN") != std::string::npos)
            {
                u = std::make_shared<logicalaccess::OmnikeyLANXX21ReaderUnit>(readerName);
            }
            else
            {
                u = std::make_shared<logicalaccess::OmnikeyXX21ReaderUnit>(readerName);
            }
        }
        else if (readerName.find("5022") != std::string::npos)
        {
            u = std::make_shared<logicalaccess::OmnikeyXX22ReaderUnit>(readerName);
        }
        else if (readerName.find("x25") != std::string::npos ||
                 readerName.find("5025-CL") != std::string::npos)
        {
            u = std::make_shared<logicalaccess::OmnikeyXX25ReaderUnit>(readerName);
        }
    }
    else if (readerName.find("SDI010 Contactless Reader") != std::string::npos ||
             readerName.find("SCR331-DI USB ContactlessReader") != std::string::npos ||
             readerName.find("SCL010 Contactless") != std::string::npos ||
             readerName.find("SCL01x Contactless") != std::string::npos ||
             readerName.find("SCL3711 reader") != std::string::npos ||
             readerName.find("Identive CLOUD 4000 F DTC CL Reader") !=
                 std::string::npos ||
             readerName.find("Identive CLOUD 4700 F Contactless") != std::string::npos ||
             readerName.find("Identive CLOUD 4710 F Contactless") != std::string::npos ||
             readerName.find("SCM Microsystems Inc. SCL011G Contactless Reader") !=
                 std::string::npos)
    {
        u = std::make_shared<logicalaccess::SCMReaderUnit>(readerName);
    }
    else if (readerName.find("Cherry ") != std::string::npos)
    {
        u = std::make_shared<logicalaccess::CherryReaderUnit>(readerName);
    }
    else if (readerName.find("SpringCard") != std::string::npos)
    {
        u = std::make_shared<logicalaccess::SpringCardReaderUnit>(readerName);
    }
    else if (readerName.find("ACS ACR1222 3S PICC Reader PICC") != std::string::npos ||
             readerName.find("ACS ACR1222 3S PICC Reader 00 00") !=
                 std::string::npos) // Name under Linux
    {
        u = std::make_shared<logicalaccess::ACSACR1222LReaderUnit>(readerName);
    }
    else if (readerName.find("ACS ACR") != std::string::npos)
    {
        u = std::make_shared<logicalaccess::ACSACRReaderUnit>(readerName);
    }
    else if (readerName.find("id3 Semiconductors") != std::string::npos)
    {
        u = std::make_shared<logicalaccess::ID3ReaderUnit>(readerName);
    }
}

LLA_READERS_PCSC_API char *getLibraryName()
{
    return (char *)"PCSC";
}

LLA_READERS_PCSC_API void
getPCSCReader(std::shared_ptr<logicalaccess::ReaderProvider> *rp)
{
    if (rp != nullptr)
    {
        *rp = logicalaccess::PCSCReaderProvider::createInstance();
    }
}

LLA_READERS_PCSC_API bool getReaderInfoAt(unsigned int index, char *readername,
                                          size_t readernamelen, void **getterfct)
{
    bool ret = false;
    if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN &&
        getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getPCSCReader;
            sprintf(readername, READER_PCSC);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}

LLA_READERS_PCSC_API void
getMifarePCSCCommands(std::shared_ptr<logicalaccess::Commands> *commands)
{
    if (commands != nullptr)
    {
        *commands = std::make_shared<logicalaccess::MifarePCSCCommands>();
    }
}
}
