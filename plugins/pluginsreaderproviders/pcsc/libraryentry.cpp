#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "pcscreaderprovider.hpp"
#include "readers/omnikeylanxx21readerunit.hpp"
#include "readers/omnikeyxx25readerunit.hpp"
#include "readers/omnikeyxx27readerunit.hpp"
#include "readers/scmreaderunit.hpp"
#include "readers/springcardreaderunit.hpp"
#include "readers/cherryreaderunit.hpp"
#include "readers/acsacrreaderunit.hpp"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C"
{

    /**
    * Attempt to create a ReaderUnit object for a reader with name readName.
    * This function returns NULL if there is no match, is does NOT create a default PCSCReaderUnit.
    */
    LIBLOGICALACCESS_API void getReaderUnit(const std::string &readerName, std::shared_ptr<logicalaccess::ReaderUnit> &u)
    {
        using namespace std;
        using namespace logicalaccess;

        if (readerName.find("OMNIKEY") != string::npos)
        {
            if (readerName.find("x21") != string::npos || readerName.find("5321") != string::npos || readerName.find("6321") != string::npos)
            {
                if (readerName.find("LAN") != string::npos)
                {
                    u = make_shared<OmnikeyLANXX21ReaderUnit>(readerName);
                }
                else
                {
                    u = make_shared<OmnikeyXX21ReaderUnit>(readerName);
                }
            }
            else if (readerName.find("x25") != string::npos || readerName.find("5025-CL") != string::npos)
            {
                u = make_shared<OmnikeyXX25ReaderUnit>(readerName);
            }
            else if (readerName.find("x27") != string::npos || readerName.find("5127") != string::npos || readerName.find("5427") != string::npos)
            {
                u = make_shared<OmnikeyXX27ReaderUnit>(readerName);
            }
        }
        else if (readerName.find("SDI010 Contactless Reader") != string::npos
                || readerName.find("SCR331-DI USB ContactlessReader") != string::npos
                || readerName.find("SCL010 Contactless") != string::npos)
        {
            u = make_shared<SCMReaderUnit>(readerName);
        }
        else if (readerName.find("Cherry ") != string::npos)
        {
            u = make_shared<CherryReaderUnit>(readerName);
        }
        else if (readerName.find("SpringCard") != string::npos)
        {
            u = make_shared<SpringCardReaderUnit>(readerName);
        }
        else if (readerName.find("ACS ACR") != string::npos)
        {
            u = make_shared<ACSACRReaderUnit>(readerName);
        }
    }

    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"PCSC";
    }

    LIBLOGICALACCESS_API void getPCSCReader(std::shared_ptr<logicalaccess::ReaderProvider>* rp)
    {
        if (rp != NULL)
        {
            *rp = logicalaccess::PCSCReaderProvider::createInstance();
        }
    }

    LIBLOGICALACCESS_API bool getReaderInfoAt(unsigned int index, char* readername, size_t readernamelen, void** getterfct)
    {
        bool ret = false;
        if (readername != NULL && readernamelen == PLUGINOBJECT_MAXLEN && getterfct != NULL)
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
            }
        }

        return ret;
    }
}