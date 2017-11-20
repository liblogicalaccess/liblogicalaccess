#include <logicalaccess/plugins/readers/stidprg/stidprgproxaccesscontrolcardservice.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgutils.hpp>
#include <logicalaccess/plugins/readers/stidprg/stidprgreaderunit.hpp>
#include <cassert>

using namespace logicalaccess;

STidPRGProxAccessControlCardService::STidPRGProxAccessControlCardService(std::shared_ptr<Chip> chip)
    : AccessControlCardService(chip)
{
}

bool STidPRGProxAccessControlCardService::writeFormat(std::shared_ptr<Format> format,
                                           std::shared_ptr<Location>,
                                           std::shared_ptr<AccessInfo>,
                                           std::shared_ptr<AccessInfo>)
{
    EXCEPTION_ASSERT_WITH_LOG(format, LibLogicalAccessException,
                              "Format cannot be null.");

    // Retrieve handle to reader.
    auto ru = getChip()
                  ->getCommands()
                  ->getReaderCardAdapter()
                  ->getDataTransport()
                  ->getReaderUnit();
    EXCEPTION_ASSERT_WITH_LOG(ru, LibLogicalAccessException,
                              "Failed to retrieve reader unit");

    auto stdiprg_ru = std::dynamic_pointer_cast<STidPRGReaderUnit>(ru);
    EXCEPTION_ASSERT_WITH_LOG(stdiprg_ru, LibLogicalAccessException,
                              "Retrieved reader unit is of wrong type.");

    stdiprg_ru->writeBlock(0, 3, STidPRGUtils::prox_configuration_bytes(*format));
    return true;
}
