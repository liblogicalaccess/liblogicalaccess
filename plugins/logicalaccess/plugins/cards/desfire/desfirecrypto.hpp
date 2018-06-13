/**
 * \file desfirecrypto.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireCrypto.
 */

#ifndef DESFIRECRYPTO_HPP
#define DESFIRECRYPTO_HPP

#include <logicalaccess/plugins/cards/desfire/desfireaccessinfo.hpp>
#include <logicalaccess/plugins/crypto/des_cipher.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <string>
#include <logicalaccess/iks/RemoteCrypto.hpp>

namespace logicalaccess
{
/**
 * \brief The cryptographic method.
 */
typedef enum {
    CM_LEGACY = 0x00, // D40
    CM_ISO    = 0x01, // EV1
    CM_EV2    = 0x02  // EV2
} CryptoMethod;

// manage desfire crypto operation through iks.
struct IKSCryptoWrapper
{
    // IKS would generate a temporary key
    // after a proper Desfire authentication (through IKS).
    // Obviously key name is non-guessable and is unique per client / auth process
    std::string remote_key_name;

    // Simply the last signature received when we call aes_decrypt() through
    // and IKSRPCClient.
    SignatureResult last_sig;
};

/**
 * \brief DESFire cryptographic functions.
 */
class LIBLOGICALACCESS_API DESFireCrypto
{
  public:
    /**
     * \brief Constructor
     */
    DESFireCrypto();

    /**
     * \brief Destructor
     */
    virtual ~DESFireCrypto();

    /**
     * \brief Decipher data step 2.
     * \param data The data buffer
     */
    void appendDecipherData(const ByteVector &data);

    /**
     * \brief Get the deciphered data into a buffer.
     * \param length The excepted deciphared data buffer length, or 0 to automatic.
     * \return data The deciphered data buffer
     */
    virtual ByteVector desfireDecrypt(size_t length);

    /**
     * \brief Verify MAC into the buffer.
     * \param end True if it's the last buffer, false otherwise
     * \param data The data buffer
     * \return True on success, false otherwise.
     */
    virtual bool verifyMAC(bool end, const ByteVector &data);

    /**
     * \brief Generate MAC for the total buffer.
     * \param data The data buffer part
     * \return The MACed data buffer
     */
    virtual ByteVector generateMAC(unsigned char cmd, const ByteVector &data);

    /**
     * \brief Encrypt a buffer for the DESFire card.
     * \param data The data buffer
     * \param param The parameters.
     * \return The encrypted data buffer
     */
    virtual ByteVector desfireEncrypt(const ByteVector &data,
                                      const ByteVector &param = ByteVector(),
                                      bool calccrc            = true);

    ByteVector iso_encipherData(bool end, const ByteVector &data,
                                const ByteVector &param = ByteVector());

    /**
     * \brief Calculate ISO14443 Type A compatible CRC-16.
     This is ITU-T V.41, not ISO 3309.
     The differences:
     - Init value is 0x6363 and not 0xffff
     - Data is not inverted at the end.
     The generating polynomial is the same, that is
     x^16 + x^12 + x^5 + 1
     * \param data The data buffer
     * \param dataLength The data buffer length
     * \return The CRC.
     */
    static short desfire_crc16(const void *data, size_t dataLength);

    /**
     * \brief Calculate CRC-32.
     * \param data The data buffer
     * \param dataLength The data buffer length
     * \return The CRC.
     */
    static uint32_t desfire_crc32(const void *data, size_t dataLength);

    /**
     * \brief Perform DESFire CBC decryption operation, which is used for authentication
     * and data encryption.
     * \param key The DES key to use
     * \param iv The Initialization Vector
     * \param data The data source buffer to decrypt
     * \return The decrypted data buffer
     */
    static ByteVector desfire_CBC_send(const ByteVector &key, const ByteVector &iv,
                                       const ByteVector &data);

    /**
     * \brief  Perform DESFire CBC "decryption" operation which is used for decrypting
     * data received from DESFire.
     * \param key The DES key to use
     * \param iv The Initialization Vector
     * \param data The data source buffer to decrypt
     * \return The decrypted data buffer
     */
    static ByteVector desfire_CBC_receive(const ByteVector &key, const ByteVector &iv,
                                          const ByteVector &data);

    /**
     * \brief  Perform DESFire CBC encryption operation, which is used for MAC calculation
     and verification.
     The algorithm is actually a standard DES CBC.
     * \param key The DES key to use
     * \param iv The Initialization Vector
     * \param data The data source buffer to encrypt
     * \return The data encrypted buffer
     */
    static ByteVector desfire_CBC_mac(const ByteVector &key, const ByteVector &iv,
                                      const ByteVector &data);

    /**
     * \brief  Preform standard CBC encryption operation, which is used for DESFire SAM
     * cryptograms.
     * \param key The DES key to use
     * \param iv The Initialization Vector
     * \param data The data source buffer to encrypt
     * \return The data encrypted buffer
     */
    static ByteVector sam_CBC_send(const ByteVector &key, const ByteVector &iv,
                                   const ByteVector &data);

    /**
     * \brief Return data with the DESFire MAC attached.
     * \param key The DES key to use, shall be the session key from the previous
     * authentication
     * \return The data mac buffer
     */
    static ByteVector desfire_mac(const ByteVector &key, ByteVector data);

    /**
     * \brief  Return data part for the encrypted communication mode for WriteData /
     * WriteRecord.
     * \param key The DES key to use, shall be the session key from the previous
     * authentication
     * \param data The data source buffer to encrypt
     * \return The data encrypted buffer
     */
    static ByteVector desfire_encrypt(const ByteVector &key, ByteVector data,
                                      bool calccrc = true);

    /**
     * \brief  Return data part for the encrypted communication mode for WriteData /
     * WriteRecord.
     * \param key The key to use, shall be the session key from the previous
     * authentication
     * \param data The data source buffer to encrypt
     * \param cipher The cypher to use
     * \param block_size The bloc size
     * \param param The optional parameters
     * \return The data encrypted buffer
     */
    ByteVector
    desfire_iso_encrypt(const ByteVector &key, const ByteVector &data,
                        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher,
                        unsigned int block_size, const ByteVector &param = ByteVector(),
                        bool calccrc = true);

    /**
     * \brief Return data part for the encrypted communication mode for WriteData /
     * WriteRecord.
     * \param key The DES key to use, shall be the session key from the previous
     * authentication
     * \param data The data source buffer to encrypt
     * \return The data encrypted buffer
     */
    static ByteVector sam_encrypt(const ByteVector &key, ByteVector data);

    /**
     * \brief Decrypt and verify data part of the decrypted communication mode for
     * ReadData / ReadRecords.
     * \param key The DES key to use, shall be the session key from the previous
     * authentication
     * \param data The data source buffer to decrypted
     * \return The data decrypted buffer
     */
    static ByteVector desfire_decrypt(const ByteVector &key, const ByteVector &data,
                                      size_t datalen);

    /**
     * \brief Decrypt and verify data part of the decrypted communication mode for
     * ReadData / ReadRecords.
     * \param key The key to use, shall be the session key from the previous
     * authentication
     * \param data The data source buffer to decrypted
     * \param cipher The cipher to use
     * \param block_size The bloc size
     * \return The data decrypted buffer
     */
    ByteVector
    desfire_iso_decrypt(const ByteVector &key, const ByteVector &data,
                        std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher,
                        unsigned int block_size, size_t datalen);

    /**
     * \brief Decrypt and verify data part of the decrypted communication mode for
     * ReadData / ReadRecords.
     * \param data The data source buffer to decrypted
     * \param length The decrypted excepted length
     * \return The data decrypted buffer
     */
    ByteVector desfire_iso_decrypt(const ByteVector &data, size_t length);

    /**
     * \brief  Return data part for the encrypted communication mode.
     * \param key The key to use, shall be the session key from the previous
     * authentication
     * \param block_size The bloc size
     * \param data The data source buffer to calculate MAC
     * \return The MAC result for the message.
     */
    ByteVector desfire_cmac(const ByteVector &key,
                            std::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC,
                            unsigned int block_size, const ByteVector &data);

    /**
     * \brief  Return data part for the encrypted communication mode.
     * \param data The data source buffer to calculate MAC
     * \return The MAC result for the message.
     */
    ByteVector desfire_cmac(const ByteVector &data);

    /**
     * \brief Authenticate on the card, step 1 for mutual authentication.
     * \param keyno The key number to use
     * \param diversify The diversify buffer for key diversification, NULL if no
     * diversification is needed
     * \param encRndB The encrypted random number B
     * \return The random number A+B 1.
     */
    virtual ByteVector authenticate_PICC1(unsigned char keyno, ByteVector diversify,
                                          const ByteVector &encRndB);

    /**
     * \brief Authenticate on the card, step 2 for mutual authentication.
     * \param keyno The key number to use
     * \param encRndA The encrypted random number A
     */
    virtual void authenticate_PICC2(unsigned char keyno, const ByteVector &encRndA);

    /**
     * \brief Authenticate on the card, step 1 for mutual authentication using ISO
     * command.
     * \param keyno The key number to use
     * \param diversify The diversify buffer for key diversification, NULL if no
     * diversification is needed
     * \param encRndB The encrypted random number B
     * \param randomlen The random length
     * \return The random number A+B 1.
     */
    ByteVector iso_authenticate_PICC1(unsigned char keyno, ByteVector diversify,
                                      const ByteVector &encRndB, unsigned int randomlen);

    /**
     * \brief Authenticate on the card, step 2 for mutual authentication using ISO
     * command.
     * \param keyno The key number to use
     * \param randomlen The random length
     */
    void iso_authenticate_PICC2(unsigned char keyno, const ByteVector &encRndA1,
                                unsigned int randomlen);

    /**
     * \brief Authenticate on the card, step 1 for mutual authentication using AES.
     * \param keyno The key number to use
     * \param diversify The diversify buffer for key diversification, NULL if no
     * diversification is needed
     * \param encRndB The encrypted random number B
     * \return The random number A+B 1.
     */
    ByteVector aes_authenticate_PICC1(unsigned char keyno, ByteVector diversify,
                                      const ByteVector &encRndB);

    /**
     * Silimar to aes_authenticate_PICC1 except it uses a generic Key rather than
     * key material as bytes.
     *
     * KEY DIVERSIFICATION IS NOT SUPPORTED.
     */
    ByteVector aes_authenticate_PICC1_GENERIC(unsigned char keyno,
                                              const std::shared_ptr<Key> &key,
                                              const ByteVector &encRndB);

    /**
     * \brief Authenticate on the card, step 2 for mutual authentication using AES.
     * \param keyno The key number to use
     * \param encRndA1 The encrypted random number A
     */
    void aes_authenticate_PICC2(unsigned char keyno, const ByteVector &encRndA1);

    void aes_authenticate_PICC2_GENERIC(unsigned char keyno,
                                        const std::shared_ptr<Key> &key, const ByteVector &encRndA1);

    /**
     * \brief Init buffer for temporised data.
     */
    void initBuf();

    /**
     * \brief Get key diversified.
     * \param key The DESFire key information
     * \param diversify The diversify buffer, NULL if no diversification is needed
     * \param keydiv The key data, diversified if a diversify buffer is specified.
     */
    static void getKey(std::shared_ptr<DESFireKey> key, ByteVector diversify,
                       ByteVector &keydiv);

    /**
     * \brief Get DES key versionned.
     * \param key The DESFire key information
     * \param keyversioned The key versioned.
     */
    static void getKeyVersioned(std::shared_ptr<DESFireKey> key,
                                ByteVector &keyversioned);

    /**
     * \brief Select an application.
     * \param aid The Application ID to select.
     */
    void selectApplication(size_t aid);

    /**
     * \brief Change key into the card.
     * \param keyno The key number to change
     * \param newkey The new key information
     * \param newKeyDiversify The diversify buffer, NULL if no diversification is needed
     * \return The change key cryptogram.
     */
    virtual ByteVector changeKey_PICC(uint8_t keyno, ByteVector oldKeyDiversify,
                                      std::shared_ptr<DESFireKey> newkey,
                                      ByteVector newKeyDiversify,
                                      unsigned char keysetno = 0);

    void setCryptoContext(ByteVector identifier);

    void createApplication(int aid, uint8_t maxKeySlotNb, uint8_t maxNbKeys,
                           DESFireKeyType cryptoMethod);

    void setIdentifier(ByteVector identifier)
    {
        d_identifier = identifier;
    }

    ByteVector getIdentifier() const
    {
        return d_identifier;
    }

    /**
    * \brief Get the default key for an algorithm.
    * \param keyType The key algorithm type.
    * \return The default key.
    */
    static std::shared_ptr<DESFireKey> getDefaultKey(DESFireKeyType keyType);

    /**
    * \brief Set default keys for the type card in memory at a specific location.
    */
    virtual void setDefaultKeysAt(std::shared_ptr<Location> location);

    /**
    * \brief Set one of the DESFire keys of this profile.
    * \param aid Application ID
    * \param keyslot The key slot to set
    * \param keyno The key number to set
    * \param key The value of the key.
    */
    virtual void setKey(size_t aid, uint8_t keyslot, uint8_t keyno,
                        std::shared_ptr<DESFireKey> key);

    /**
    * \brief Set one of the DESFire keys of this keyset.
    * \param aid Application ID
    * \param keySlotNb The key slot to set
    * \param nbKeys The key number to set
    * \param key The value of the key.
    */
    virtual void setKeyInAllKeySet(size_t aid, uint8_t keySlotNb, uint8_t nbKeys,
                                   std::shared_ptr<DESFireKey> key);

    /**
    * \brief Clear all keys in memory.
    */
    virtual void clearKeys();

    /**
    * \brief Get a key in memory from the current application.
    * \param keyslot The key slot.
    * \param keyno The key number.
    * \return The key.
    */
    std::shared_ptr<DESFireKey> getKey(uint8_t keyslot, uint8_t keyno) const;

  protected:
    /**
    * \brief Get key from memory.
    * \param aid The Application ID
    * \param keyno The key number
    * \param diversify The diversify buffer, NULL if no diversification is needed
    * \param keydiv The key data, diversified if a diversify buffer is specified.
    * \return True on success, false otherwise.
    */
    bool getKey(size_t aid, uint8_t keyset, uint8_t keyno, ByteVector diversify,
                ByteVector &keydiv);

    /**
    * \brief Get key from memory.
    * \param keyno The key number
    * \param diversify The diversify buffer, NULL if no diversification is needed
    * \param keydiv The key data, diversified if a diversify buffer is specified.
    * \return True on success, false otherwise.
    */
    bool getKey(uint8_t keyset, uint8_t keyno, ByteVector diversify, ByteVector &keydiv);

    /**
    * \brief Get one of the DESFire keys of this profile.
    * \param aid The application id.
    * \param keyslot The key slot to set
    * \param keyno The key number to set
    * \return The specified DESFire key or a null key if params are invalid.
    */
    std::shared_ptr<DESFireKey> getKey(size_t aid, uint8_t keyslot, uint8_t keyno) const;

    /**
     * Key store - AID / KeySlot / KeyNo / DESFireKey
     */
    std::map<std::tuple<size_t, uint8_t, uint8_t>, std::shared_ptr<DESFireKey>> d_keys;

  public:
    /**
     * \brief The current authentication method.
     */
    CryptoMethod d_auth_method;

    /**
     * \brief The current cipher.
     */
    std::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher;

    /**
     * \brief The MAC size.
     */
    unsigned int d_mac_size;

    /**
     * \brief The block size.
     */
    unsigned int d_block_size;

    /**
     * \brief The current session key.
     */
    ByteVector d_sessionKey;

    /**
     * \brief The authentication key.
     */
    ByteVector d_authkey;

    /**
     * \brief The last Initialization Vector (DESFire native mode).
     */
    ByteVector d_lastIV;

    /**
     * \brief The current Application ID.
     */
    unsigned int d_currentAid;

    /**
     * \brief The current Key number.
     */
    unsigned char d_currentKeyNo;

#ifndef SWIG
    // If present it means we use IKS...
    std::unique_ptr<IKSCryptoWrapper> iks_wrapper_;
#endif

    /**
     * Retrieve the IKS signature (if requested) for the last decrypted
     * data block.
     *
     * This will return an empty object if no IKS was used or if the operation
     * mode does make sense to have that.
     */
    SignatureResult get_last_signature() const;

  protected:
    /**
     * \brief The temporised buffer.
     */
    ByteVector d_buf;

    /**
     * \brief The last left buffer for card command.
     */
    ByteVector d_last_left;

    /**
     * \brief The random number A.
     */
    ByteVector d_rndA;

    /**
     * \brief The random number B.
     */
    ByteVector d_rndB;

    /**
     * \brief The card identifier use for key diversification.
     */
    ByteVector d_identifier;
};
}

#endif /* DESFIRECRYPTO_HPP */