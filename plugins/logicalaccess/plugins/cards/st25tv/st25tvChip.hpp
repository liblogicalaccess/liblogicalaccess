/**
 * \file st25tvChip.hpp
 * \author Rémi W <remi.wickuler@islog.com>
 * \brief st25tv chip.
 */

#pragma once
#include <logicalaccess/plugins/cards/st25tv/lla_cards_st25tv_api.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693chip.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
  #define CHIP_ST25TV "ST25TV"

  class LLA_CARDS_ST25TV_API  St25tvChip : public ISO15693Chip
  {
  public:
    St25tvChip();
    explicit St25tvChip(std::string ct);
    ~St25tvChip();
  /*  std::shared_ptr<ISO15693Commands> getISO15693Commands() const
    {
        return std::dynamic_pointer_cast<ISO15693Commands>(getCommands());
    }*/
  };
}
