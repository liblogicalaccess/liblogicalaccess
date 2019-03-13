#pragma once

#include <cstdint>
#include <logicalaccess/plugins/crypto/lla_crypto_api.hpp>
#include <logicalaccess/plugins/crypto/symmetric_cipher.hpp>
#include <logicalaccess/plugins/crypto/sha.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_CRYPTO_API ISO24727Crypto
{
  public:
    explicit ISO24727Crypto(const std::string& cipher = "3des", const std::string& hash = "sha1");

    /**
     * Call after construction to feed the random from the
     * card to the crypto helper.
     *
     * The random_ifd and random_k_ifd array will be generated if left
     * to they default, empty value. Specifying this parameter
     * is useful for testing purpose or if you make use of a custom
     * source of randomness.
     *
     * @return Command data for Mutual Authenticate.
     */
    ByteVector step1(const ByteVector &random_icc, ByteVector random_ifd = {},
                     ByteVector random_k_ifd = {});

    /**
     * Process the response from the ICC.
     *
     * If step2() completes successfully, the user is allowed to
     * call the various get_*() method.
     */
    bool step2(const ByteVector &auth_response);

    /**
     * Are we currently already in a Secure Messaging session ?
     */
    bool secureMode() const;

    ByteVector encrypt_apdu(const ByteVector &apdu);

    ByteVector decrypt_rapdu(const ByteVector &rapdu);

	ByteVector encrypt_apdu(std::shared_ptr<openssl::SymmetricCipher> cipher,
                            const ByteVector &apdu, const ByteVector &ks_enc,
                            const ByteVector &ks_mac, const ByteVector &ssc);

    ByteVector decrypt_rapdu(std::shared_ptr<openssl::SymmetricCipher> cipher,
                             const ByteVector &rapdu, const ByteVector &ks_enc,
                             const ByteVector &ks_mac, const ByteVector &ssc);

    ByteVector get_session_enc_key() const;
    ByteVector get_session_mac_key() const;
    ByteVector get_send_session_counter() const;

	virtual ByteVector auth_pad(const ByteVector &data);

	virtual void compute_session_keys(const ByteVector &k_icc,
                                        const ByteVector &random_icc) = 0;

	virtual ByteVector compute_mac(std::shared_ptr<openssl::SymmetricCipher> cipher,
                                       const ByteVector &in, const ByteVector &k_mac,
                                       const ByteVector &iv  = {},
                                       const ByteVector &scc = {}) = 0;

	virtual void reset();

	std::shared_ptr<openssl::SymmetricCipher> createCipher() const;

    /**
        * ISO 9797-1 padding method 2
        */
    static ByteVector pad(const ByteVector &in, unsigned int padlen = 8);

    /**
        * Cancel the padding created by pad()
        */
    static ByteVector unpad(const ByteVector &in);

    /**
        * Increment the Session Send Counter by one and return a new
        * ByteVector.
        */
    static ByteVector increment_ssc(const ByteVector &in);

    static ByteVector hash_data(const ByteVector &data,
                                const std::string hash_algo = "sha1");


	ByteVector k_enc_;
    ByteVector k_mac_;

  protected:
	std::string cipher_;
    std::string hash_;
    /**
     * Generated at step1 (or inputted at step1).
     */
    ByteVector random_ifd_;
    /**
     * Generated at step1 (or inputted at step1).
     */
    ByteVector random_k_ifd_;

    bool step2_success_;

    ByteVector S_enc_;
    ByteVector S_mac_;
    ByteVector S_send_counter_;
};
}
