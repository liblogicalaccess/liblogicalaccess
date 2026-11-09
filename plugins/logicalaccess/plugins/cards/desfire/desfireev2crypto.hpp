#pragma once

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>

namespace logicalaccess
{
/**
 * Collection of function to help with DESFire crypto operation.
 */
class LLA_CARDS_DESFIRE_API DESFireEV2Crypto : public DESFireCrypto
{
  public:
    DESFireEV2Crypto()
        : d_macSessionKey()
        , ti_()
        , cmdctr_()
    {
    }

    /**
     * @param rndb: challenge from the reader
     * @param rnda: random from us.
     */
    static ByteVector get_challenge_response(ByteVector rndb, const ByteVector &rnda);

    static ByteVector rotateAuthenticationChallenge(const ByteVector &rnd);

    struct AuthenticationResponse
    {
        ByteVector ti;
        ByteVector piccCap2;
        ByteVector pcdCap2;
    };

    /**
     * \brief Parse and verify the 32-byte EV2 First response.
     * Capability echo validation is optional for legacy DESFire callers.
     */
    static AuthenticationResponse parseEV2FirstResponse(
        const ByteVector &data, const ByteVector &rnda,
        const ByteVector &expectedPCDCap2 = ByteVector(), bool validateCapabilities = false);

    /**
     * \brief Derive AES-128 or AES-256 EV2 keys without changing session state.
     * Card-specific entry points remain responsible for restricting supported key types.
     * \return Session encryption key followed by the session MAC key.
     */
    static std::pair<ByteVector, ByteVector> deriveEV2SessionKeys(
        const ByteVector &key, const ByteVector &rnda, const ByteVector &rndb);

    /**
     * \brief Install a First session, replacing prior authentication state and resetting the counter.
     */
    void installFirstSession(const std::pair<ByteVector, ByteVector> &keys,
                             const ByteVector &ti, CryptoMethod method, std::uint8_t keyno);

    /**
     * \brief Replace NonFirst session keys while preserving TI, counter and authentication mode.
     */
    void installNonFirstSession(const std::pair<ByteVector, ByteVector> &keys, std::uint8_t keyno);

    /**
     * Finalize the AuthEV2 process.
     *
     * @param data is the decrypted 32-byte data sent by the chip.
     * @return the Transaction Identifier.
     */
    static ByteVector auth_ev2_part2(const ByteVector &data, const ByteVector &rnda);

    /**
     * \brief Legacy AES-128 derivation/installation entry point; resets the counter without replacing TI.
     */
    void generateSessionKey(const ByteVector &key_data, const ByteVector &rnda, const ByteVector &rndb);

    void sam_generateSessionKey(const ByteVector &rnda, const ByteVector &rndb, std::shared_ptr<SAMAV2ISO7816Commands> cmd);

    void setTI(const ByteVector &ti)
    {
        ti_ = ti;
    }

    void setCmdCtr(std::uint16_t i)
    {
        cmdctr_ = i;
    }

    std::uint16_t getCmdCtr() const
    {
        return cmdctr_;
    }

    /**
     * \brief Verify MAC into the buffer.
     * \param end True if it's the last buffer, false otherwise
     * \param data The data buffer
     * \return True on success, false otherwise.
     */
    bool verifyMAC(bool end, const ByteVector &data) override;

    /**
     * \brief Generate MAC for the total buffer.
     * \param data The data buffer part
     * \return The MACed data buffer
     */
    ByteVector generateMAC(unsigned char cmd, const ByteVector &data) override;

    /**
     * \brief Encrypt a buffer for the DESFire card.
     * \param data The data buffer
     * \param param The parameters.
     * \return The encrypted data buffer
     */
    ByteVector desfireEncrypt(const ByteVector &data,
                              const ByteVector &param = ByteVector(),
                              bool calccrc            = true) override;

    /**
     * \brief Get the deciphered data into a buffer.
     * \param length The excepted deciphered data buffer length, or 0 to automatic.
     * \return data The deciphered data buffer
     */
    ByteVector desfireDecrypt(std::size_t length) override;

    ByteVector changeKey_PICC(unsigned char keyno, ByteVector oldKeyDiversify,
                              std::shared_ptr<DESFireKey> newkey,
                              ByteVector newKeyDiversify,
                              unsigned char keysetno = 0) override;

    void duplicateCurrentKeySet(std::uint8_t keySetNb);

    void duplicateKeySet(std::uint8_t keySetNb, std::uint8_t keySetNbToDuplicate);

    void setKeySetCryptoType(std::uint8_t keySetNb, DESFireKeyType cryptoMethod);

    /**
     * \brief The current mac session key.
     */
    ByteVector d_macSessionKey;

    static ByteVector truncateMAC(const ByteVector &full_mac);

    /**
     * \brief Invalidates the current authentication session
     *
     * Clears all authentication and secure-messaging state, including session keys, authentication nonces,
     * transaction state, and secure-messaging chaining state
     *
     * The application selection and configured key store are preserved
     *
     * This must be called whenever the current authentication session is no longer valid and must not be reused
     */
    void invalidateAuthentication();
    
    /**
     * \brief Initialize the EV2-compatible secure-messaging state.
     *
     * Resets the command counter and initializes the AES cipher and MAC size.
     */
    void initializeSecureMessagingState();

  private:
    ByteVector getIVEncrypt(bool cmdData);

    static std::tuple<ByteVector, ByteVector> getSV(const ByteVector &rnda, const ByteVector &rndb);

    static ByteVector buildKDFContext(const ByteVector &rnda, const ByteVector &rndb);
    static ByteVector buildKDFVector(std::uint8_t label0, std::uint8_t label1,
                                    std::uint16_t counter, std::uint16_t length,
                                    const ByteVector &context);
    void replaceSessionKeys(const std::pair<ByteVector, ByteVector> &keys);

    ByteVector ti_;
    std::uint16_t cmdctr_;
};
} // namespace logicalaccess
