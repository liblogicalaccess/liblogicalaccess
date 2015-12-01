#pragma once

#include "logicalaccess/cardprobe.hpp"

namespace logicalaccess
{
class PCSCCardProbe : public CardProbe
{
  public:
    PCSCCardProbe(ReaderUnit *ru);

  public:
    virtual bool is_desfire(std::vector<uint8_t> *uid) override;

    virtual bool is_desfire_ev1(std::vector<uint8_t> *uid) override;

    virtual bool maybe_mifare_classic() override;

  protected:
    void reset();
};
}