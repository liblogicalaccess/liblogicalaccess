/**
 * \file desfireev2commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief DESFire EV2 commands.
 */

#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2chip.hpp>

namespace logicalaccess
{
std::shared_ptr<DESFireEV2Chip> DESFireEV2Commands::getDESFireEV2Chip() const
{
    return std::dynamic_pointer_cast<DESFireEV2Chip>(getChip());
}
} // namespace logicalaccess