/**
 * \file pcscreaderprovider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Reader Proviser PC/SC.
 */

#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>

#if defined(__unix__)
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>

#include <iomanip>

#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
PCSCReaderProvider::PCSCReaderProvider()
    : ISO7816ReaderProvider()
{
    d_scc      = 0;
    long scres = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &d_scc);
    if (scres != SCARD_S_SUCCESS)
    {
        char tmpbuf[128];
        memset(tmpbuf, 0x00, sizeof(tmpbuf));
        sprintf(tmpbuf, "Can't establish the context for PC/SC service (%x).",
                static_cast<unsigned int>(scres));
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmpbuf);
    }
}

PCSCReaderProvider::~PCSCReaderProvider()
{
    PCSCReaderProvider::release();
}

void PCSCReaderProvider::release()
{
    if (d_scc != 0)
    {
        SCardReleaseContext(d_scc);

        d_scc = 0;
    }
}

std::shared_ptr<PCSCReaderProvider> PCSCReaderProvider::createInstance()
{
    std::shared_ptr<PCSCReaderProvider> ret(new PCSCReaderProvider());
    ret->refreshReaderList();
    return ret;
}

bool PCSCReaderProvider::refreshReaderList()
{
    bool r = false;
    d_system_readers.clear();

    DWORD rdlen = 0;
    if (SCARD_S_SUCCESS == SCardListReaders(d_scc, nullptr, (char *)nullptr, &rdlen))
    {
        char *rdnames = new char[rdlen];
        if (SCARD_S_SUCCESS == SCardListReaders(d_scc, nullptr, rdnames, &rdlen))
        {
            char *rdname = rdnames;

            while (rdname[0] != '\0')
            {
                size_t f = strlen(rdname);
                std::string t(rdname, f);
                std::shared_ptr<PCSCReaderUnit> unit =
                    PCSCReaderUnit::createPCSCReaderUnit(t);
                unit->setReaderProvider(
                    std::weak_ptr<ReaderProvider>(shared_from_this()));
                d_system_readers.push_back(unit);

                rdname += strlen(rdname) + 1;
            }

            r = true;
        }
        delete[] rdnames;
    }

    return r;
}

std::shared_ptr<ReaderUnit> PCSCReaderProvider::createReaderUnit()
{
    // return createReaderUnit("Generic PCSC ReaderUnit");
    return createReaderUnit("");
}

std::shared_ptr<ISO7816ReaderUnit>
PCSCReaderProvider::createReaderUnit(std::string readerunitname)
{
    std::shared_ptr<PCSCReaderUnit> ret = PCSCReaderUnit::createPCSCReaderUnit(readerunitname);
    ret->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));

    return ret;
}

std::vector<std::string> PCSCReaderProvider::getReaderGroupList() const
{
    std::vector<std::string> groupList;

    DWORD rdlen = 0;
    if (SCARD_S_SUCCESS == SCardListReaderGroups(d_scc, (char *)nullptr, &rdlen))
    {
        char *rdnames = new char[rdlen];
        if (SCARD_S_SUCCESS == SCardListReaderGroups(d_scc, rdnames, &rdlen))
        {
            char *rdname = rdnames;

            while (rdname[0] != '\0')
            {
                size_t f = strlen(rdname);
                std::string t(rdname, f);
                groupList.push_back(t);

                rdname += strlen(rdname) + 1;
            }
        }
        delete[] rdnames;
    }

    return groupList;
}
}