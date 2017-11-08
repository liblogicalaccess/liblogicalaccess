#pragma once

#include <logicalaccess/cardprobe.hpp>

namespace logicalaccess
{
class LIBLOGICALACCESS_API PCSCCardProbe : public CardProbe
{
  public:
    explicit PCSCCardProbe(ReaderUnit *ru);

    bool is_desfire(ByteVector *uid = nullptr) override;

    bool is_desfire_ev1(ByteVector *uid = nullptr) override;

    bool is_desfire_ev2(ByteVector *uid = nullptr) override;

    bool is_mifare_ultralight_c() override;

    bool maybe_mifare_classic() override;

    bool has_desfire_random_uid(ByteVector *uid) override;

  protected:
    void reset() const;

  private:
    unsigned char get_desfire_version(ByteVector *uid = nullptr);
};
}
