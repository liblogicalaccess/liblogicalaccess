/**
 * \file st25tvChip.cpp
 * \author Rémi W <remi.wickuler@islog.com>
 * \brief st25tv chip.
 */

#include <logicalaccess/plugins/cards/st25tv/st25tvChip.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
  St25tvChip::St25tvChip() : ISO15693Chip(CHIP_ST25TV)
  {
  }

  St25tvChip::St25tvChip(std::string ct) : ISO15693Chip(ct)
  {

  }

  St25tvChip::~St25tvChip()
  {

  }
}
