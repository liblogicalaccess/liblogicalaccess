/**
 * \file resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief Result checker base class.
 */

#include "logicalaccess/resultchecker.hpp"

namespace logicalaccess
{
	ResultChecker::ResultChecker()
	{
	}

	ResultChecker::~ResultChecker()
	{
		mapPrimary.clear();
		mapSingleShot.clear();
	}

	void ResultChecker::CheckResult(const void* data, size_t datalen)
	{
		ResultCheckerValues result("", false);

		if (!mapPrimary.empty() || !mapSingleShot.empty())
		{
			if (datalen >= 2)
			{
				unsigned char SW1 = reinterpret_cast<const unsigned char*>(data)[datalen - 2];
				unsigned char SW2 = reinterpret_cast<const unsigned char*>(data)[datalen - 1];

				ResultCheckerKeys keys(SW1, SW2);

				if (!mapSingleShot.empty())
				{
					auto res = mapSingleShot.find(keys);
					if (res != mapSingleShot.end())
					{
						result = res->second;
					}
				}

				if (!result.second && !mapPrimary.empty())
				{
					auto res = mapPrimary.find(keys);
					if (res != mapPrimary.end())
					{
						result = res->second;
					}
				}
			}
		}

		mapSingleShot.clear();

		if (result.second && !result.first.empty())
		{
			THROW_EXCEPTION_WITH_LOG(CardException, result.first);
		}
		else if (!result.first.empty())
		{
			LOG(LogLevel::WARNINGS) << result.first;
		}
	}

	void ResultChecker::AddCheck(unsigned char SW1, unsigned char SW2, const std::string& msg, bool throwException)
	{
		ResultCheckerKeys keys(SW1, SW2);
		ResultCheckerValues values(msg, throwException);

		mapPrimary[keys] = values;
	}

	void ResultChecker::RemoveCheck(unsigned char SW1, unsigned char SW2)
	{
		ResultCheckerKeys keys(SW1, SW2);

		auto res = mapPrimary.find(keys);

		if (res != mapPrimary.end())
		{
			mapPrimary.erase(keys);
		}
	}

	void ResultChecker::ClearChecks()
	{
		mapPrimary.clear();
	}

	void ResultChecker::AddSingleShotCheck(unsigned char SW1, unsigned char SW2, const std::string& msg)
	{
		ResultCheckerKeys keys(SW1, SW2);
		ResultCheckerValues values(msg, true);

		mapSingleShot[keys] = values;
	}
}