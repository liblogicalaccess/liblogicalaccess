/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 */
#include "logicalaccess/crypto/tomcrypt.h"

/**
  @file crypt_cipher_descriptor.c
  Stores the cipher descriptor table, Tom St Denis
  */

struct ltc_cipher_descriptor cipher_descriptor[TAB_SIZE] = {
        {nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
};

LTC_MUTEX_GLOBAL(ltc_cipher_mutex)

/* $Source$ */
/* $Revision$ */
/* $Date$ */