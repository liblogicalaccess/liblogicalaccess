#pragma once

#include <logicalaccess/cardprobe.hpp>
#include <logicalaccess/plugins/readers/pcsc/lla_readers_pcsc_api.hpp>

namespace logicalaccess
{
class LLA_READERS_PCSC_API PCSCCardProbe : public CardProbe
{
  public:
    struct DESFireVersionInfo
    {
        int hardwareMajorVersion = -1;
        int softwareMajorVersion = -1;
        ByteVector uid;
    };

    explicit PCSCCardProbe(ReaderUnit *ru);

    bool is_desfire(ByteVector *uid = nullptr) override;

    bool is_desfire_ev1(ByteVector *uid = nullptr) override;

    bool is_desfire_ev2(ByteVector *uid = nullptr) override;

    bool is_desfire_ev3(ByteVector *uid = nullptr) override;

    bool is_mifare_ultralight_c() override;

    bool maybe_mifare_classic() override;

    bool has_desfire_random_uid(ByteVector *uid) override;

    DESFireVersionInfo get_desfire_version();

  protected:
    void reset() const;

  private:
    DESFireVersionInfo probe_desfire_version();
};

} // namespace logicalaccess
