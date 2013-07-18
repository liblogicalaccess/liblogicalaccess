/**
 * \file desfirecrypto.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireCrypto.
 */

#ifndef SAMAV2CRYPTO_HPP
#define SAMAV2CRYPTO_HPP

#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"


#ifndef UNIX
#include "logicalaccess/msliblogicalaccess.h"
#endif

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace logicalaccess
{
	class LIBLOGICALACCESS_API SAMAV2Crypto
	{
		public:
			/**
			 * \brief Constructor
			 */
			SAMAV2Crypto();

			/**
			 * \brief Destructor
			 */
			virtual ~SAMAV2Crypto();

		

		static std::vector<unsigned char> SAMAV2Crypto::desfire_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

		static std::vector<unsigned char> SAMAV2Crypto::desfire_CBC_mac(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

		static std::vector<unsigned char> SAMAV2Crypto::sam_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data);

		static std::vector<unsigned char> SAMAV2Crypto::desfire_mac(const std::vector<unsigned char>& key, std::vector<unsigned char> data);

		static short SAMAV2Crypto::desfire_crc16(const void* data, size_t dataLength);

		static std::vector<unsigned char> SAMAV2Crypto::desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data);

		public:

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
			 * \brief The card identifier use for key diversification.
			 */
			std::vector<unsigned char> d_identifier;
	};	
}

#endif /* DESFIRECRYPTO_HPP */
