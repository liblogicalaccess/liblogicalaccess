#pragma once

#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>

namespace logicalaccess
{
class DESFireEV3Chip;

/**
 * \brief The DESFire EV3 base commands class.
 *
 * We do not reimplement or add anything yet.
 */
class LLA_CARDS_DESFIRE_API DESFireEV3Commands : public DESFireEV2Commands
{
  public:
    // We inherit everything from EV2.
    // We are backward compat mode only.
    ~DESFireEV3Commands() override;

  private:
    std::shared_ptr<DESFireEV3Chip> getDESFireEV3Chip() const;
};

}
