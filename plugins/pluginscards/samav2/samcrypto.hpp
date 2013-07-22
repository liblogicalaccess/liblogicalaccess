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

			virtual std::vector<unsigned char> authenticate_PICC1(unsigned char  keyno, unsigned char* diversify, const std::vector<unsigned char>& encRndB);

			virtual void authenticate_PICC2(unsigned char  keyno, const std::vector<unsigned char>& encRndA);
	};	
}

#endif
