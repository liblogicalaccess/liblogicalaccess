/**
 * \file st25tvChip.cpp
 * \author Rémi W <remi.wickuler@islog.com>
 * \brief st25tv commands.
 */

#pragma once
#include <logicalaccess/plugins/cards/iso15693/iso15693commands.hpp>
#include <logicalaccess/plugins/cards/st25tv/st25tvChip.hpp>

namespace logicalaccess
{
  #define CMD_ST25TV "ST25TV"

  class LLA_CARDS_ST25TV_API St25tvCommands : public ISO15693Commands
  {
  public:
    St25tvCommands();
    explicit St25tvCommands(std::string ct);
    virtual void stayQuiet();
    virtual ByteVector readBlock(size_t block, size_t le);
    virtual void writeBlock(size_t block, const ByteVector &data);
    virtual void lockBlock(size_t block);
    virtual void writeAFI(size_t afi);
    virtual void lockAFI();
    virtual void writeDSFID(size_t dsfid);
    virtual void lockDSFID();
    virtual SystemInformation getSystemInformation();
    virtual unsigned char getSecurityStatus(size_t block);
  protected:
    virtual std::shared_ptr<St25tvChip> getSt25tvChip();
  private:
    SystemInformation si;
  };
}
