/**
 * \file PCSCReaderProvider.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Reader Proviser PC/SC.
 */

#include "PCSCReaderProvider.h"

#ifdef __linux__
#include <stdlib.h>
#include <sys/time.h>
#endif

#include <sstream>

using std::ostringstream;
using std::istringstream;

#include <iomanip>

using std::hex;
using std::setw;
using std::setfill;

#include "PCSCReaderUnit.h"


namespace logicalaccess
{
	PCSCReaderProvider::PCSCReaderProvider() :
		ReaderProvider()
	{
		d_scc = 0;
		long scres = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &d_scc);
		if (scres != SCARD_S_SUCCESS)
		{
			char tmpbuf[128];
			memset(tmpbuf, 0x00, sizeof(tmpbuf));
			sprintf(tmpbuf, "Can't establish the context for PC/SC service (%x).", static_cast<unsigned int>(scres));
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, tmpbuf);
		}
	}

	PCSCReaderProvider::~PCSCReaderProvider()
	{
		if (d_scc != 0)
		{
			SCardReleaseContext(d_scc);

			d_scc = 0;
		}
	}

	boost::shared_ptr<PCSCReaderProvider> PCSCReaderProvider::createInstance()
	{
		boost::shared_ptr<PCSCReaderProvider> ret(new PCSCReaderProvider());
		ret->refreshReaderList();
		return ret;
	}

	bool PCSCReaderProvider::refreshReaderList()
	{
		bool r = false;
		d_system_readers.clear();

		DWORD rdlen = 0;
		if (SCARD_S_SUCCESS == SCardListReaders(d_scc, NULL, (char*)NULL, &rdlen))
		{
			char* rdnames = new char[rdlen];
			if (SCARD_S_SUCCESS == SCardListReaders(d_scc, NULL, rdnames, &rdlen))
			{
				char* rdname = rdnames;

				while (rdname[0] != '\0')
				{
					size_t f = strlen(rdname);
					string t(rdname, f);
					boost::shared_ptr<PCSCReaderUnit> unit = PCSCReaderUnit::createPCSCReaderUnit(t);
					unit->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));
					d_system_readers.push_back(unit);

					rdname += strlen(rdname) + 1;
				}

				r = true;
			}
			delete[] rdnames;
		}

		return r;
	}

	boost::shared_ptr<ReaderUnit> PCSCReaderProvider::createReaderUnit()
	{
		boost::shared_ptr<PCSCReaderUnit> ret;
		ret.reset(new PCSCReaderUnit(""));
		ret->setReaderProvider(boost::weak_ptr<ReaderProvider>(shared_from_this()));

		return ret;
	}

	std::vector<std::string> PCSCReaderProvider::getReaderGroupList()
	{
		std::vector<std::string> groupList;

		DWORD rdlen = 0;
		if (SCARD_S_SUCCESS == SCardListReaderGroups(d_scc, (char*)NULL, &rdlen))
		{
			char* rdnames = new char[rdlen];
			if (SCARD_S_SUCCESS == SCardListReaderGroups(d_scc, rdnames, &rdlen))
			{
				char* rdname = rdnames;

				while (rdname[0] != '\0')
				{
					size_t f = strlen(rdname);
					string t(rdname, f);
					groupList.push_back(t);

					rdname += strlen(rdname) + 1;
				}
			}
			delete[] rdnames;
		}

		return groupList;
	}
}

