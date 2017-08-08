#pragma once

#include "logicalaccess/cardprobe.hpp"

namespace logicalaccess
{
class LIBLOGICALACCESS_API PCSCCardProbe : public CardProbe
{
  public:
    PCSCCardProbe(ReaderUnit *ru);

  public:
    virtual bool is_desfire(std::vector<uint8_t> *uid = nullptr) override;

	virtual bool is_desfire_ev1(std::vector<uint8_t> *uid = nullptr) override;

	virtual bool is_desfire_ev2(std::vector<uint8_t> *uid = nullptr) override;

	virtual bool is_mifare_ultralight_c() override;

    virtual bool maybe_mifare_classic() override;

    virtual bool has_desfire_random_uid(ByteVector *uid) override;

  protected:
    void reset();

	private:
		unsigned char get_desfire_version(std::vector<uint8_t> *uid = nullptr);
};
}
