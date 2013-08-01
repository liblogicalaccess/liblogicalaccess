/**
 * \file desfirecrypto.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireCrypto.
 */

#ifndef SAMDESFIRECRYPTO_HPP
#define SAMDESFIRECRYPTO_HPP

#include "logicalaccess/crypto/des_cipher.hpp"
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/cards/profile.hpp"
#include "desfirecrypto.hpp"


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
	/**
	 * \brief SAM cryptographic functions.
	 */
	class LIBLOGICALACCESS_API SAMDESfireCrypto : public DESFireCrypto
	{
		public:
			/**
			 * \brief Constructor
			 */
			SAMDESfireCrypto();

			/**
			 * \brief Destructor
			 */
			virtual ~SAMDESfireCrypto();

			static std::vector<unsigned char> desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data);
	};	
}

#endif
