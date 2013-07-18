/**
 * \file desfirecrypto.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire cryptographic functions.
 */

#include "samav2crypto.hpp"
#include "logicalaccess/crypto/tomcrypt.h"
#include <zlib.h>
#include <ctime>
#include <cstdlib>

#ifndef UNIX
#include "logicalaccess/msliblogicalaccess.h"
#endif

#include <cstring>

#include <openssl/rand.h>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"
#include "logicalaccess/crypto/des_symmetric_key.hpp"
#include "logicalaccess/crypto/des_initialization_vector.hpp"


namespace logicalaccess
{		
	SAMAV2Crypto::SAMAV2Crypto()
	{
		//d_auth_method = CM_LEGACY;
		d_currentAid = 0;
		d_currentKeyNo = 0;
		d_mac_size = 4;
		d_block_size = 8;

		d_lastIV.clear();
		d_lastIV.resize(d_block_size, 0x00);
	}

	SAMAV2Crypto::~SAMAV2Crypto()
	{
		
	}

	
	

	std::vector<unsigned char> SAMAV2Crypto::desfire_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		symmetric_key skey;
		unsigned char in[8], out[8], in2[8];
		
		unsigned int i=0;
		unsigned int j=0;

		std::vector<unsigned char> ret;

		bool is3des = false;
		if (memcmp(&key[0], &key[8], 8))
		{
			is3des = true;
		}

		// Set encryption keys
		if (is3des)
		{
			des3_setup(&key[0], 16, 0, &skey);
		}
		else
		{
			des_setup(&key[0], 8, 0, &skey);
		}
	

		// clear buffers
		memset(in, 0x00, 8);
		memset(out, 0x00, 8);
		memset(in2, 0x00, 8);

		// do for each 8 byte block of input
		for (i=0; i<data.size()/8; i++)
		{
			// copy 8 bytes from input buffer to in
			memcpy(in, &data[i*8], 8);

			if (i == 0)
			{
				if (iv.size() >= 8)
				{
					for (j=0;j<8;j++)
					{
						in[j] ^=  iv[j];
					}
				}
			}
			else
			{
				for (j=0;j<8;j++)
				{
					in[j] ^= in2[j];
				}
			}
			
			// encryption
			if (is3des)
			{
				des3_ecb_decrypt(in, out, &skey);			
			}
			else
			{
				des_ecb_decrypt(in, out, &skey);
			}

			memcpy(in2, out, 8);

			// copy decrypted block to output
			ret.insert(ret.end(), out, out + 8);
		}

		if (is3des)
		{
			des3_done(&skey);		
		}
		else
		{
			des_done(&skey);
		}

		return ret;
	}

	std::vector<unsigned char> SAMAV2Crypto::desfire_encrypt(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
	{
		int pad = (8 - ((data.size() + 2) % 8)) % 8;

		short crc = desfire_crc16(&data[0], data.size());
		data.push_back(static_cast<unsigned char>(crc & 0xff));
		data.push_back(static_cast<unsigned char>((crc & 0xff00) >> 8));
		for (int i = 0; i < pad; ++i)
		{
			data.push_back(0x00);
		}

		return sam_CBC_send(key, std::vector<unsigned char>(), data);
	}

	short SAMAV2Crypto::desfire_crc16(const void* data, size_t dataLength)
	{
		unsigned char first, second;

		ComputeCrc(CRC_A, reinterpret_cast<const unsigned char*>(data), dataLength, &first, &second);

		return ((second << 8) | first);
	}

	std::vector<unsigned char> SAMAV2Crypto::desfire_CBC_mac(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		std::vector<unsigned char> ret = sam_CBC_send(key, iv, data);
		return std::vector<unsigned char>(ret.end() - 8, ret.end() - 4);
	}

	std::vector<unsigned char> SAMAV2Crypto::sam_CBC_send(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv, const std::vector<unsigned char>& data)
	{
		unsigned char in[8], out[8];
		symmetric_key skey;
		unsigned int i=0;
		unsigned int j=0;

		std::vector<unsigned char> ret;

		des3_setup(&key[0], 16, 0, &skey);

		// clear buffers
		memset(in, 0x00, 8);
		memset(out, 0x00, 8);

		// do for each 8 byte block of input
		for (i=0; i < data.size()/8; i++)
		{
			// copy 8 bytes from input buffer to in
			memcpy(in, &data[i*8], 8);

			if (i == 0)
			{
				if (iv.size() >= 8)
				{
					for (j=0;j<8;j++)
					{
						in[j] ^= iv[j];
					}
				}
			}
			else
			{
				for (j=0;j<8;j++)
				{
					in[j] = in[j]^out[j];					
				}
			}
			
			// 3DES encryption
			des3_ecb_encrypt(in, out, &skey);

			// copy encrypted block to output
			ret.insert(ret.end(), out, out + 8);
		}

		des3_done(&skey);

		return ret;
	}

	std::vector<unsigned char> SAMAV2Crypto::desfire_mac(const std::vector<unsigned char>& key, std::vector<unsigned char> data)
	{
		int pad = (8 - (data.size() % 8)) % 8;
		for (int i = 0; i < pad; ++i)
		{
			data.push_back(0x00);
		}

		return desfire_CBC_mac(key, std::vector<unsigned char>(), data);
	}

}

