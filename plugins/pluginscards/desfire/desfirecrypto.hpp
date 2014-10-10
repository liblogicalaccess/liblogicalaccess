/**
 * \file desfirecrypto.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireCrypto.
 */

#ifndef DESFIRECRYPTO_HPP
#define DESFIRECRYPTO_HPP

#include "desfireaccessinfo.hpp"
#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "desfireprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
    /**
     * \brief The cryptographic method.
     */
    typedef enum
    {
        CM_LEGACY = 0x00,
        CM_ISO = 0x01
    }CryptoMethod;

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
         * \brief Decipher data step 1.
         * \param length The total buffer length of all steps
         * \param data The data buffer
         */
        void decipherData1(size_t length, const std::vector<unsigned char>& data);

        /**
         * \brief Decipher data step 2.
         * \param data The data buffer
         */
        void decipherData2(const std::vector<unsigned char>& data);

        /**
         * \brief Get the deciphered data into a buffer.
         * \param length The excepted deciphared data buffer length, or 0 to automatic.
         * \return data The deciphered data buffer
         */
        std::vector<unsigned char> decipherData(size_t length);

        /**
         * \brief Verify MAC into the buffer.
         * \param end True if it's the last buffer, false otherwise
         * \param data The data buffer
         * \return True on success, false otherwise.
         */
        bool verifyMAC(bool end, const std::vector<unsigned char>& data);

        /**
         * \brief Add the buffer to the temporised total buffer.
         * \param data The data buffer part
         */
        void bufferingForGenerateMAC(const std::vector<unsigned char>& data);

        /**
         * \brief Generate MAC for the total buffer.
         * \param data The data buffer part
         * \return The MACed data buffer
         */
        std::vector<unsigned char> generateMAC(const std::vector<unsigned char>& data);

        /**
         * \brief Encrypt a buffer for the DESFire card.
         * \param data The data buffer
         * \param param The parameters.
         * \return The encrypted data buffer
         */
        std::vector<unsigned char> desfireEncrypt(const std::vector<unsigned char>& data, const std::vector<unsigned char>& param = std::vector<unsigned char>());

        /**
         * \brief Encipher a data buffer part, and temporised it.
         * \param end True if it's the last buffer part.
         * \param data The data buffer
         * \return The encrypted data buffer
         */
        std::vector<unsigned char> encipherData(bool end, std::vector<unsigned char> data, const std::vector<unsigned char>& param = std::vector<unsigned char>());

        std::vector<unsigned char> iso_encipherData(bool end, const std::vector<unsigned char>& data, const std::vector<unsigned char>& param = std::vector<unsigned char>());

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
        static short desfire_crc16(const void* data, size_t dataLength);

        /**
         * \brief Calculate CRC-32.
         * \param data The data buffer
         * \param dataLength The data buffer length
         * \return The CRC.
         */
        static long desfire_crc32(const void* data, size_t dataLength);

        /**
         * \brief Perform DESFire CBC decryption operation, which is used for authentication and data encryption.
         * \param key The DES key to use
         * \param iv The Initialization Vector
         * \param data The data source buffer to decrypt
         * \return The decrypted data buffer
         */
        static std::vector<unsigned char> desfire_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

        /**
         * \brief  Perform DESFire CBC "decryption" operation which is used for decrypting data received from DESFire.
         * \param key The DES key to use
         * \param iv The Initialization Vector
         * \param data The data source buffer to decrypt
         * \return The decrypted data buffer
         */
        static std::vector<unsigned char> desfire_CBC_receive(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

        /**
         * \brief  Perform DESFire CBC encryption operation, which is used for MAC calculation and verification.
         The algorithm is actually a standard DES CBC.
         * \param key The DES key to use
         * \param iv The Initialization Vector
         * \param data The data source buffer to encrypt
         * \return The data encrypted buffer
         */
        static std::vector<unsigned char> desfire_CBC_mac(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

        /**
         * \brief  Preform standard CBC encryption operation, which is used for DESFire SAM cryptograms.
         * \param key The DES key to use
         * \param iv The Initialization Vector
         * \param data The data source buffer to encrypt
         * \return The data encrypted buffer
         */
        static std::vector<unsigned char> sam_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

        /**
         * \brief Return data with the DESFire MAC attached.
         * \param key The DES key to use, shall be the session key from the previous authentication
         * \param dataSrc The data source buffer to attach MAC
         * \param dataSrcLength The data source buffer length
         * \return The data mac buffer
         */
        std::vector<unsigned char> desfire_mac(const std::vector<unsigned char>& key, std::vector<unsigned char> data);

        /**
         * \brief  Return data part for the encrypted communication mode for WriteData / WriteRecord.
         * \param key The DES key to use, shall be the session key from the previous authentication
         * \param data The data source buffer to encrypt
         * \return The data encrypted buffer
         */
        static std::vector<unsigned char> desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data);

        /**
         * \brief  Return data part for the encrypted communication mode for WriteData / WriteRecord.
         * \param key The key to use, shall be the session key from the previous authentication
         * \param data The data source buffer to encrypt
         * \param cipher The cypher to use
         * \param block_size The bloc size
         * \param param The optional parameters
         * \return The data encrypted buffer
         */
        std::vector<unsigned char> desfire_iso_encrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher, unsigned int block_size, const std::vector<unsigned char>& param = std::vector<unsigned char>());

        /**
         * \brief Return data part for the encrypted communication mode for WriteData / WriteRecord.
         * \param key The DES key to use, shall be the session key from the previous authentication
         * \param data The data source buffer to encrypt
         * \return The data encrypted buffer
         */
        std::vector<unsigned char> sam_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data);

        /**
         * \brief Decrypt and verify data part of the decrypted communication mode for ReadData / ReadRecords.
         * \param key The DES key to use, shall be the session key from the previous authentication
         * \param data The data source buffer to decrypted
         * \return The data decrypted buffer
         */
        std::vector<unsigned char> desfire_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, size_t datalen);

        /**
         * \brief Decrypt and verify data part of the decrypted communication mode for ReadData / ReadRecords.
         * \param key The key to use, shall be the session key from the previous authentication
         * \param data The data source buffer to decrypted
         * \param cipher The cipher to use
         * \param block_size The bloc size
         * \param length The decrypted excepted length
         * \return The data decrypted buffer
         */
        std::vector<unsigned char> desfire_iso_decrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipher, unsigned int block_size, size_t datalen);

        /**
         * \brief Decrypt and verify data part of the decrypted communication mode for ReadData / ReadRecords.
         * \param data The data source buffer to decrypted
         * \param length The decrypted excepted length
         * \return The data decrypted buffer
         */
        std::vector<unsigned char> desfire_iso_decrypt(const std::vector<unsigned char>& data, size_t length);

        /**
         * \brief  Return data part for the encrypted communication mode.
         * \param key The key to use, shall be the session key from the previous authentication
         * \param cipher The cypher to use
         * \param block_size The bloc size
         * \param data The data source buffer to calculate MAC
         * \return The MAC result for the message.
         */
        std::vector<unsigned char> desfire_cmac(const std::vector<unsigned char>& key, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data);

        /**
         * \brief  Return data part for the encrypted communication mode.
         * \param data The data source buffer to calculate MAC
         * \return The MAC result for the message.
         */
        std::vector<unsigned char> desfire_cmac(const std::vector<unsigned char>& data);

        /**
         * \brief Authenticate on the card, step 1 for mutual authentication.
         * \param keyno The key number to use
         * \param diversify The diversify buffer for key diversification, NULL if no diversification is needed
         * \param encRndB The encrypted random number B
         * \return The random number A+B 1.
         */
        virtual std::vector<unsigned char> authenticate_PICC1(unsigned char  keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB);

        /**
         * \brief Authenticate on the card, step 2 for mutual authentication.
         * \param keyno The key number to use
         * \param encRndA The encrypted random number A
         */
        virtual void authenticate_PICC2(unsigned char  keyno, const std::vector<unsigned char>& encRndA);

        /**
         * \brief Authenticate on the card, step 1 for mutual authentication using ISO command.
         * \param keyno The key number to use
         * \param diversify The diversify buffer for key diversification, NULL if no diversification is needed
         * \param encRndB The encrypted random number B
         * \param randomlen The random length
         * \return The random number A+B 1.
         */
        std::vector<unsigned char> iso_authenticate_PICC1(unsigned char keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB, unsigned int randomlen);

        /**
         * \brief Authenticate on the card, step 2 for mutual authentication using ISO command.
         * \param keyno The key number to use
         * \param encRndA The encrypted random number A
         * \param randomlen The random length
         */
        void iso_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1, unsigned int randomlen);

        /**
         * \brief Authenticate on the card, step 1 for mutual authentication using AES.
         * \param keyno The key number to use
         * \param diversify The diversify buffer for key diversification, NULL if no diversification is needed
         * \param encRndB The encrypted random number B
         * \return The random number A+B 1.
         */
        std::vector<unsigned char> aes_authenticate_PICC1(unsigned char keyno, std::vector<unsigned char> diversify, const std::vector<unsigned char>& encRndB);

        /**
         * \brief Authenticate on the card, step 2 for mutual authentication using AES.
         * \param keyno The key number to use
         * \param encRndA The encrypted random number A
         */
        void aes_authenticate_PICC2(unsigned char keyno, const std::vector<unsigned char>& encRndA1);

        /**
         * \brief Init buffer for temporised data.
         * \param length The data length.
         */
        void initBuf(size_t length);

        /**
         * \brief Get key diversified.
         * \param key The DESFire key information
         * \param diversify The diversify buffer, NULL if no diversification is needed
         * \param keydiv The key data, diversified if a diversify buffer is specified.
         */
        static void getKey(boost::shared_ptr<DESFireKey> key, std::vector<unsigned char> diversify, std::vector<unsigned char>& keydiv);

        /**
         * \brief Get DES key versionned.
         * \param key The DESFire key information
         * \param keyversioned The key versioned.
         */
        static void getKeyVersioned(boost::shared_ptr<DESFireKey> key, std::vector<unsigned char>& keyversioned);

        /**
         * \brief Select an application.
         * \param aid The Application ID to select.
         */
        void selectApplication(size_t aid);

        /**
         * \brief Change key into the card.
         * \param keyno The key number to change
         * \param newKey The new key information
         * \param diversify The diversify buffer, NULL if no diversification is needed
         * \return The change key cryptogram.
         */
        std::vector<unsigned char> changeKey_PICC(unsigned char keyno, boost::shared_ptr<DESFireKey> newKey, std::vector<unsigned char> diversify);

        void setCryptoContext(boost::shared_ptr<DESFireProfile> profile, std::vector<unsigned char> identifier);

        /**
         * \brief Get the diversify buffer.
         * \param diversify The diversify buffer.
         * \return True on success, false otherwise.
         */
        bool getDiversify(unsigned char* diversify);

        /**
         * \brief Get a key in memory from the current application.
         * \param keyno The key number.
         * \return The key.
         */
        boost::shared_ptr<DESFireKey> getKey(unsigned char keyno);

        void createApplication(int aid, size_t maxNbKeys, DESFireKeyType cryptoMethod);

        const std::vector<unsigned char> getIdentifier() const { return d_identifier; };

    public:

        /**
         * \brief The current authentication method.
         */
        CryptoMethod d_auth_method;

        /**
         * \brief The current cipher.
         */
        boost::shared_ptr<openssl::OpenSSLSymmetricCipher> d_cipher;

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
        std::vector<unsigned char> d_sessionKey;

        /**
         * \brief The authentication key.
         */
        std::vector<unsigned char> d_authkey;

        /**
         * \brief The last Initialization Vector (DESFire native mode).
         */
        std::vector<unsigned char> d_lastIV;

        /**
         * \brief The current Application ID.
         */
        int d_currentAid;

        /**
         * \brief The current Key number.
         */
        unsigned char d_currentKeyNo;

    protected:

        /**
         * \brief The temporised buffer.
         */
        std::vector<unsigned char> d_buf;

        /**
         * \brief The last left buffer for card command.
         */
        std::vector<unsigned char> d_last_left;

        /**
         * \brief The random number A.
         */
        std::vector<unsigned char> d_rndA;

        /**
         * \brief The random number B.
         */
        std::vector<unsigned char> d_rndB;

    protected:

        /**
         * \brief The DESFire profile which own keys.
         */
        boost::shared_ptr<DESFireProfile> d_profile;

        /**
         * \brief The card identifier use for key diversification.
         */
        std::vector<unsigned char> d_identifier;
    };
}

#endif /* DESFIRECRYPTO_HPP */