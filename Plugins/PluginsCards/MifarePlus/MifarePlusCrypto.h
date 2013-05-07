/**
 * \file MifarePlusCrypto.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusCrypto.
 */

#ifndef MIFAREPLUSCRYPTO_H
#define MIFAREPLUSCRYPTO_H

#include "MifarePlusAccessInfo.h"
#include "MifarePlusProfile.h"
#include "../DESFire/DESFireCrypto.h"

#include "logicalaccess/crypto/des_cipher.h"
#include "logicalaccess/crypto/aes_cipher.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

/**
* \ List of crypto usefull defines
*/
#define MIFARE_PLUS_MAC_SIZE		0x08

namespace LOGICALACCESS
{
	/**
	 * \brief DESFire cryptographic functions.
	 */
	class LIBLOGICALACCESS_API MifarePlusCrypto
	{
		public:
			/**
			 * \brief Constructor
			 */
			MifarePlusCrypto();

			/**
			 * \brief Destructor
			 */
			~MifarePlusCrypto();

			/**
			 * \brief Calculate Kmac using rndA and rndB with the current authenticated key
			 */
			void calculateKmac();

			/**
			 * \brief Calculate Kenc using rndA and rndB with the current authenticated key
			 */
			void calculateKenc();

			/**
			 * \brief Calculate MAC to put on the command
			 * \param command The command to send to the card
			 */
			std::vector<unsigned char> GetMacOnCommand(std::vector<unsigned char> command);

			/**
			 * \brief Calculate MAC that must be on the reponse
			 * \param response The card response
			 * \param command The command sent to the card
			 */
			std::vector<unsigned char> GetMacOnResponse(std::vector<unsigned char> response, std::vector<unsigned char> command);

			/**
			* \brief Generic AES unciper method
			* \param in The key to uncipher
			* \param keyString The AES cipher key
			* \param expectedSize The expected size of the unciphered key
			* \param ivSL3 The specific SL3 identification vector indicator (used in ciphered commands and responses)
			*/
			std::vector<unsigned char> AESUncipher(std::vector<unsigned char> in, std::vector<unsigned char> keyString, bool ivSL3);
		
			/**
			* \brief Generic AES cipher method
			* \param in The key to cipher
			* \param keyString The AES cipher key
			* \param ivSL3 The specific SL3 identification vector indicator (used in ciphered commands and responses)
			*/
			std::vector<unsigned char> AESCipher(std::vector<unsigned char> in, std::vector<unsigned char> keyString, bool ivSL3);

			/**
			* \brief Get th initialisation vector counter part string
			*/
			std::vector<unsigned char> GetIvStringCountPart();

		public:

			/**
			 * \brief The current session encryption key.
			 */
			std::vector<unsigned char> d_Kenc;

			/**
			 * \brief The current session MAC key.
			 */
			std::vector<unsigned char> d_Kmac;

			/**
			 * \brief The authentication key.
			 */
			std::vector<unsigned char> d_AESauthkey;

			/**
			 * \brief The random number A.
			 */
			std::vector<unsigned char> d_rndA;
			
			/**
			 * \brief The random number B.
			 */
			std::vector<unsigned char> d_rndB;

			/**
			* \brief The Transaction Identifier
			*/
			std::vector<unsigned char> d_ti;

			/**
			* \brief The PICC Capabilities
			*/
			std::vector<unsigned char> d_PICCcap;

			/**
			* \brief The PCD capabilities
			*/
			std::vector<unsigned char> d_PCDcap;

			/**
			* \brief The counter for writing operations (used in MAC)
			*/
			unsigned short d_Wctr;

			/**
			* \brief The counter for deading operations (used in MAC)
			*/
			unsigned short d_Rctr;
	};	
}

#endif /* MIFAREPLUSCRYPTO_H */
