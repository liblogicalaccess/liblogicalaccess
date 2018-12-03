#pragma once

#include <cstdint>
#include <logicalaccess/plugins/cards/epass/lla_cards_epass_api.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
class LLA_CARDS_EPASS_API EPassCrypto
{
  public:
    explicit EPassCrypto(const std::string &mrz);

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

    ByteVector get_session_enc_key() const;
    ByteVector get_session_mac_key() const;
    ByteVector get_send_session_counter() const;

  private:
    /**
     * Generated at step1 (or inputted at step1).
     */
    ByteVector random_ifd_;
    /**
     * Generated at step1 (or inputted at step1).
     */
    ByteVector random_k_ifd_;

    bool step2_success_;
    ByteVector k_enc_;
    ByteVector k_mac_;

    ByteVector S_enc_;
    ByteVector S_mac_;
    ByteVector S_send_counter_;
};
}
