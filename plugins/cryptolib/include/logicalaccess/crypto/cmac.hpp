#include <vector>
#include "boost/shared_ptr.hpp"
#include "logicalaccess/crypto/openssl_symmetric_cipher.hpp"

#ifndef CMAC_HPP__
#define CMAC_HPP__

namespace logicalaccess
{
	namespace openssl
	{
		class CMACCrypto
		{
		public:
			CMACCrypto() {};
			~CMACCrypto() {};

			/**
			 * \brief  Return data part for the encrypted communication mode.
			 * \param key The key to use, shall be the session key from the previous authentication
			 * \param cipher The cypher to use
			 * \param block_size The bloc size
			 * \param data The data source buffer to calculate MAC
			 * \param lastIV The last initialisation vector
			 * \return The MAC result for the message.
			 */
			static std::vector<unsigned char> cmac(const std::vector<unsigned char>& key, boost::shared_ptr<openssl::OpenSSLSymmetricCipher> cipherMAC, unsigned int block_size, const std::vector<unsigned char>& data, std::vector<unsigned char> lastIV, unsigned int padding_size);

			/**
			 * \brief Shift a string.
			 * \param buf The buffer string
			 * \param xorparam The optional xor for the string.
			 */
			static std::vector<unsigned char> shift_string(const std::vector<unsigned char>& buf, unsigned char xorparam = 0x00);

		private:

		};
	}
}

#endif