#pragma once

#include <logicalaccess/plugins/cards/epass/lla_cards_epass_api.hpp>
#include <logicalaccess/plugins/crypto/iso24727crypto.hpp>

namespace logicalaccess
{
class LLA_CARDS_EPASS_API EPassCrypto : public ISO24727Crypto
{
  public:
    explicit EPassCrypto();
    explicit EPassCrypto(const std::string &mrz);

	void reset(const std::string &mrz);

	static ByteVector adjust_key_parity(const ByteVector &key);

	void compute_session_keys(const ByteVector &k_icc,
                                  const ByteVector &random_icc) override;

	ByteVector compute_enc_key(const ByteVector &seed,
                                   const std::string hash_algo = "sha1",
                                   unsigned char keylen        = 16);

    ByteVector compute_mac_key(const ByteVector &seed,
                                const std::string hash_algo = "sha1",
                                unsigned char keylen        = 16);

	/**
	 * ISO 9797-1 MAC Algorithm 3
	 *
     * Perform MAC computation on the block `in`.
     *
     * The computation requires `k_mac`, a 16bytes key that will be split in two
     * during the computation.
     *
     * Unless the MAC computation is for the Mutual Authenticate command, SSC (Send
     * Session Counter) is required, otherwise the MAC will be rejected.
     *
     * @warning No padding is performed.
     */
    ByteVector compute_mac(std::shared_ptr<openssl::SymmetricCipher> cipher,
                           const ByteVector &in, const ByteVector &k_mac,
                           const ByteVector &iv  = {},
                           const ByteVector &ssc = {}) override;
};
}
