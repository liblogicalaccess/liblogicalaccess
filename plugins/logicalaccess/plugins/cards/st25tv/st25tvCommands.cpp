/**
 * \file st25tvChip.cpp
 * \author Rémi W <remi.wickuler@islog.com>
 * \brief st25tv chip.
 */

#pragma once
#include <logicalaccess/plugins/cards/st25tv/st25tvCommands.hpp>

namespace logicalaccess
{
  St25tvCommands::St25tvCommands() : ISO15693Commands(CMD_ST25TV)
  {

  }

  St25tvCommands::St25tvCommands(std::string ct) : ISO15693Commands(ct)
  {

  }

  void St25tvCommands::stayQuiet()
  {

  }

  ByteVector St25tvCommands::readBlock(size_t block, size_t le = 0)
  {
    ByteVector result;
    return result;
  }

  void St25tvCommands::writeBlock(size_t block, const ByteVector &data)
  {

  }

  void St25tvCommands::lockBlock(size_t block)
  {

  }

  void St25tvCommands::writeAFI(size_t afi)
  {

  }

  void St25tvCommands::lockAFI()
  {

  }

  void St25tvCommands::writeDSFID(size_t dsfid)
  {

  }

  void St25tvCommands::lockDSFID()
  {

  }

  St25tvCommands::SystemInformation St25tvCommands::getSystemInformation()
  {
    return si;
  }

  unsigned char St25tvCommands::getSecurityStatus(size_t block)
  {
    unsigned char status = 0;
    return status;
  }
  std::shared_ptr<St25tvChip> St25tvCommands::getSt25tvChip()
  {
    return nullptr;
  }
}
