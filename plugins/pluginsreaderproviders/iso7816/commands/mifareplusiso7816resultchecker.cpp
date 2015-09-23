//
// Created by xaqq on 6/16/15.
//

#include "mifareplusiso7816resultchecker.hpp"
#include <cassert>
#include <logicalaccess/logs.hpp>
#include "logicalaccess/myexception.hpp"

logicalaccess::MifarePlusISO7816ResultChecker::MifarePlusISO7816ResultChecker()
{

}

void logicalaccess::MifarePlusISO7816ResultChecker::CheckResult(const void *data,
                                                                size_t datalen)
{
    assert(datalen);
    checkByte(static_cast<const uint8_t *>(data)[0]);
}

bool logicalaccess::MifarePlusISO7816ResultChecker::checkByte(uint8_t t)
{
    switch (t)
    {
        case 0x06:
        THROW_EXCEPTION_WITH_LOG(CardException, "Access conditions not fulfilled.");
            break;
        case 0x07:
        THROW_EXCEPTION_WITH_LOG(CardException, "Too many read or write commands in the session or in the transaction");
            break;
        case 0x08:
            THROW_EXCEPTION_WITH_LOG(CardException, "Invalid MAC in command or response");
            break;
        case 0x09:
        THROW_EXCEPTION_WITH_LOG(CardException, "Block number is not valid");
            break;
        case 0x0A:
        THROW_EXCEPTION_WITH_LOG(CardException, "Block number is not valid / not existing");
            break;
        case 0x0B:
        THROW_EXCEPTION_WITH_LOG(CardException, "The current command code is not available at the current card state");
            break;
        case 0x0C:
        THROW_EXCEPTION_WITH_LOG(CardException, "Length error");
            break;
        case 0x0F:
        THROW_EXCEPTION_WITH_LOG(CardException, "General Manipulation Error: Failure in the operation of the PICC");
            break;
        case 0x90:
            return true;
    }
    return false;
}
