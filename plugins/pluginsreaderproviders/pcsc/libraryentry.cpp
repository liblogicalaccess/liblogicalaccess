#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "pcscreaderprovider.hpp"
#include "readers/omnikeylanxx21readerunit.hpp"
#include "readers/omnikeyxx22readerunit.hpp"
#include "readers/omnikeyxx23readerunit.hpp"
#include "readers/omnikeyxx25readerunit.hpp"
#include "readers/scmreaderunit.hpp"
#include "readers/springcardreaderunit.hpp"
#include "readers/cherryreaderunit.hpp"
#include "readers/acsacrreaderunit.hpp"
#include "readers/acsacr1222lreaderunit.hpp"
#include "readers/id3readerunit.hpp"
#include "logicalaccess/logicalaccess_api.hpp"
#include "commands/mifarepcsccommands.hpp"
#include "logicalaccess/logs.hpp"

extern "C"
{

    /**
    * Attempt to create a ReaderUnit object for a reader with name readName.
    * This function returns NULL if there is no match, is does NOT create a default PCSCReaderUnit.
    */
    LIBLOGICALACCESS_API void getReaderUnit(const std::string &readerName, std::shared_ptr<logicalaccess::ReaderUnit> &u)
    {
        if (readerName.find("OMNIKEY") != std::string::npos)
        {
            if (readerName.find("x21") != std::string::npos || readerName.find("5321") != std::string::npos || readerName.find("6321") != std::string::npos)
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
            else if (readerName.find("5023") != std::string::npos)
            {
                u = std::make_shared<logicalaccess::OmnikeyXX23ReaderUnit>(readerName);
            }
            else if (readerName.find("x25") != std::string::npos || readerName.find("5025-CL") != std::string::npos)
            {
                u = std::make_shared<logicalaccess::OmnikeyXX25ReaderUnit>(readerName);
            }
        }
        else if (readerName.find("SDI010 Contactless Reader") != std::string::npos
                || readerName.find("SCR331-DI USB ContactlessReader") != std::string::npos
                || readerName.find("SCL010 Contactless") != std::string::npos
				|| readerName.find("SCL01x Contactless") != std::string::npos
				|| readerName.find("SCL3711 reader") != std::string::npos
				|| readerName.find("Identive CLOUD 4000 F DTC CL Reader") != std::string::npos
				|| readerName.find("Identive CLOUD 4700 F Contactless") != std::string::npos
				|| readerName.find("Identive CLOUD 4710 F Contactless") != std::string::npos
				|| readerName.find("SCM Microsystems Inc. SCL011G Contactless Reader") != std::string::npos
				)
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
        else if (readerName.find("ACS ACR1222 3S PICC Reader PICC") != std::string::npos
                || readerName.find("ACS ACR1222 3S PICC Reader 00 00") != std::string::npos) // Name under Linux
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

    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"PCSC";
    }

    LIBLOGICALACCESS_API void getPCSCReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != nullptr)
        {
            *rp = logicalaccess::PCSCReaderProvider::createInstance();
        }
    }

    LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char* readername, size_t readernamelen, void** getterfct)
    {
        bool ret = false;
        if (readername != nullptr && readernamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
        {
            switch (index)
            {
            case 0:
            {
                *getterfct = (void*)&getPCSCReader;
                sprintf(readername, READER_PCSC);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }

	LIBLOGICALACCESS_API void getMifarePCSCCommands(std::shared_ptr<logicalaccess::Commands>* commands)
	{
		if (commands != nullptr)
		{
			*commands = std::make_shared<logicalaccess::MifarePCSCCommands>();
		}
	}
}
