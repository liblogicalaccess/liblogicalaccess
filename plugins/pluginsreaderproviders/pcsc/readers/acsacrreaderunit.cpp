/**
 * \file acsacrreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ACS ACR reader unit.
 */

#include "../readers/acsacrreaderunit.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "../commands/acsacrresultchecker.hpp"

namespace logicalaccess
{
ACSACRReaderUnit::ACSACRReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
}

ACSACRReaderUnit::~ACSACRReaderUnit()
{
}

PCSCReaderUnitType ACSACRReaderUnit::getPCSCType() const
{
    return PCSC_RUT_ACS_ACR;
}

std::string ACSACRReaderUnit::getFirmwareVersion()
{
    std::shared_ptr<ResultChecker> checker =
        getDefaultPCSCReaderCardAdapter()->getResultChecker();
    getDefaultPCSCReaderCardAdapter()->setResultChecker(std::shared_ptr<ResultChecker>());
    ByteVector res =
        getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x48, 0x00, 0x00);
    getDefaultPCSCReaderCardAdapter()->setResultChecker(checker);
    return BufferHelper::getStdString(res);
}

std::shared_ptr<ResultChecker> ACSACRReaderUnit::createDefaultResultChecker() const
{
    return std::make_shared<ACSACRResultChecker>();
}
}