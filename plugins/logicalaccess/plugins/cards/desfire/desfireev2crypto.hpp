#pragma once

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

    /**
     * Finalize the AuthEV2 process.
     *
     * @param data is the encrypted data sent by the chip.
     * @return the Transaction Identifier.
     */
    static ByteVector auth_ev2_part2(const ByteVector &data, const ByteVector &rnda);

    void generateSessionKey(const ByteVector &key_data, const ByteVector &rnda,
                            const ByteVector &rndb);

    void sam_generateSessionKey(const ByteVector &rnda, const ByteVector &rndb,
                                std::shared_ptr<SAMAV2ISO7816Commands> cmd);

    void setTI(const ByteVector &ti)
    {
        ti_ = ti;
    }

    void setCmdCtr(uint16_t i)
    {
        cmdctr_ = i;
    }

    uint16_t getCmdCtr() const
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
    ByteVector desfireDecrypt(size_t length) override;

    ByteVector changeKey_PICC(unsigned char keyno, ByteVector oldKeyDiversify,
                              std::shared_ptr<DESFireKey> newkey,
                              ByteVector newKeyDiversify,
                              unsigned char keysetno = 0) override;

    void duplicateCurrentKeySet(uint8_t keySetNb);

    void duplicateKeySet(uint8_t keySetNb, uint8_t keySetNbToDuplicate);

    void setKeySetCryptoType(uint8_t keySetNb, DESFireKeyType cryptoMethod);

    /**
     * \brief The current mac session key.
     */
    ByteVector d_macSessionKey;

    static ByteVector truncateMAC(const ByteVector &full_mac);

  private:
    ByteVector getIVEncrypt(bool cmdData);

    static std::tuple<ByteVector, ByteVector> getSV(const ByteVector &rnda,
                                                    const ByteVector &rndb);

    ByteVector ti_;
    uint16_t cmdctr_;
};
} // namespace logicalaccess
