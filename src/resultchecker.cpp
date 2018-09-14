/**
 * \file resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief Result checker base class.
 */

#include <logicalaccess/resultchecker.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <cassert>

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

void ResultChecker::CheckResult(const void *data, size_t datalen)
{
    ResultCheckerValues result("", CardException::DEFAULT, false);

    if (!mapPrimary.empty() || !mapSingleShot.empty())
    {
        if (datalen >= 2)
        {
            unsigned char SW1 =
                reinterpret_cast<const unsigned char *>(data)[datalen - 2];
            unsigned char SW2 =
                reinterpret_cast<const unsigned char *>(data)[datalen - 1];

            ResultCheckerKeys keys(SW1, SW2);

            if (!mapSingleShot.empty())
            {
                auto res = mapSingleShot.find(keys);
                if (res != mapSingleShot.end())
                {
                    result = res->second;
                }
            }

            if (!std::get<2>(result) && !mapPrimary.empty())
            {
                auto res = mapPrimary.find(keys);
                if (res != mapPrimary.end())
                {
                    result = res->second;
                }
            }
        }
        else
            THROW_EXCEPTION_WITH_LOG(CardException, "SW1 or SW2 is missing.");
    }

    mapSingleShot.clear();

    if (std::get<2>(result) && !std::get<0>(result).empty())
        THROW_EXCEPTION_WITH_LOG(CardException, std::get<0>(result), std::get<1>(result));

    if (!std::get<0>(result).empty())
    {
        LOG(LogLevel::WARNINGS) << std::get<0>(result);
    }
}

void ResultChecker::AddCheck(unsigned char SW1, unsigned char SW2, const std::string &msg,
                             CardException::ErrorType error /* = DEFAULT */,
                             bool throwException)
{
    ResultCheckerKeys keys(SW1, SW2);
    ResultCheckerValues values(msg, error, throwException);

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

void ResultChecker::AddSingleShotCheck(unsigned char SW1, unsigned char SW2,
                                       const std::string &msg)
{
    ResultCheckerKeys keys(SW1, SW2);
    ResultCheckerValues values(msg, CardException::DEFAULT, true);

    mapSingleShot[keys] = values;
}

bool ResultChecker::AllowEmptyResult() const
{
    return false;
}
}