#include <logicalaccess/plugins/cards/desfire/desfireev3commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev3chip.hpp>

namespace logicalaccess
{
DESFireEV3Commands::~DESFireEV3Commands() {}

std::shared_ptr<DESFireEV3Chip> DESFireEV3Commands::getDESFireEV3Chip() const
{
    return std::dynamic_pointer_cast<DESFireEV3Chip>(getChip());
}

}
