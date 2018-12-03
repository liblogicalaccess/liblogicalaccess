#ifndef TOMCRYPT_H_
#define TOMCRYPT_H_
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include "logicalaccess/plugins/crypto/lla_crypto_api.hpp"

// We keep the TomCrypt define/macro to use other TomCrypt algorithm in further time
#define LTC_NO_CIPHERS
#define LTC_DES
#define LTC_NO_HASHES
#define LTC_NO_MACS
#define LTC_NO_PRNGS
#define LTC_NO_PK
#define LTC_NO_PKCS
#define LTC_NO_PROTOTYPES

/* use configuration data */
#include <logicalaccess/plugins/crypto/tomcrypt_custom.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* version */
#define CRYPT   0x0117
#define SCRYPT  "1.17"

    /* max size of either a cipher/hash block or symmetric key [largest of the two] */
#define MAXBLOCKSIZE  128

    /* descriptor table size */
#define TAB_SIZE      32

    /* error codes [will be expanded in future releases] */
    enum {
        CRYPT_OK = 0,             /* Result OK */
        CRYPT_ERROR,            /* Generic Error */
        CRYPT_NOP,              /* Not a failure but no operation was performed */

        CRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
        CRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
        CRYPT_FAIL_TESTVECTOR,  /* Algorithm failed test vectors */

        CRYPT_BUFFER_OVERFLOW,  /* Not enough space for output */
        CRYPT_INVALID_PACKET,   /* Invalid input packet given */

        CRYPT_INVALID_PRNGSIZE, /* Invalid number of bits for a PRNG */
        CRYPT_ERROR_READPRNG,   /* Could not read enough from PRNG */

        CRYPT_INVALID_CIPHER,   /* Invalid cipher specified */
        CRYPT_INVALID_HASH,     /* Invalid hash specified */
        CRYPT_INVALID_PRNG,     /* Invalid PRNG specified */

        CRYPT_MEM,              /* Out of memory */

        CRYPT_PK_TYPE_MISMATCH, /* Not equivalent types of PK keys */
        CRYPT_PK_NOT_PRIVATE,   /* Requires a private PK key */

        CRYPT_INVALID_ARG,      /* Generic invalid argument */
        CRYPT_FILE_NOTFOUND,    /* File Not Found */

        CRYPT_PK_INVALID_TYPE,  /* Invalid type of PK key */
        CRYPT_PK_INVALID_SYSTEM,/* Invalid PK system specified */
        CRYPT_PK_DUP,           /* Duplicate key already in key ring */
        CRYPT_PK_NOT_FOUND,     /* Key not found in keyring */
        CRYPT_PK_INVALID_SIZE,  /* Invalid size input for PK parameters */

        CRYPT_INVALID_PRIME_SIZE,/* Invalid size of prime requested */
        CRYPT_PK_INVALID_PADDING /* Invalid padding on input */
    };

#include <logicalaccess/plugins/crypto/tomcrypt_cfg.h>
#include <logicalaccess/plugins/crypto/tomcrypt_macros.h>
#include <logicalaccess/plugins/crypto/tomcrypt_cipher.h>
    /*#include <tomcrypt_hash.h>
    #include <tomcrypt_mac.h>
    #include <tomcrypt_prng.h>
    #include <tomcrypt_pk.h>
    #include <tomcrypt_math.h>
    #include <tomcrypt_misc.h>*/
#include <logicalaccess/plugins/crypto/tomcrypt_argchk.h>
    /*#include <tomcrypt_pkcs.h>*/

#define CRC_A 1
#define CRC_B 2

    unsigned short UpdateCrc(unsigned char ch, unsigned short *lpwCrc);

    LLA_CRYPTO_API void ComputeCrc(int CRCType, const unsigned char *Data,
                                        size_t Length, unsigned char *TransmitFirst,
                                        unsigned char *TransmitSecond);

   LLA_CRYPTO_API  void ComputeCrcCCITT(unsigned short crc_old,
                                              const unsigned char *Data, size_t Length,
                                              unsigned char *TransmitFirst,
                                              unsigned char *TransmitSecond);

   LLA_CRYPTO_API unsigned short UpdateCRCKermit(unsigned short crc, char c);

    LLA_CRYPTO_API void ComputeCrcKermit(const unsigned char *Data, size_t Length,
                                             unsigned char *TransmitFirst,
                                             unsigned char *TransmitSecond);

   /**
    * Convert a TomCrypt error code to const char *
    */
   LLA_CRYPTO_API const char *error_to_string(int errcode);

#ifdef __cplusplus
}
#endif

#endif /* TOMCRYPT_H_ */

/* $Source$ */
/* $Revision$ */
/* $Date$ */