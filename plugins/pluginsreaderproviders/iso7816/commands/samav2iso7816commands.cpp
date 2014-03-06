/**
 * \file SAMAV2ISO7816Commands.cpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMAV2ISO7816Commands commands.
 */

#include "../readercardadapters/iso7816readercardadapter.hpp"
#include "../commands/samav2iso7816commands.hpp"
#include "../iso7816readerunitconfiguration.hpp"
#include "samcrypto.hpp"
#include "samkeyentry.hpp"
#include "samkucentry.hpp"
#include <openssl/rand.h>
#include "logicalaccess/crypto/symmetric_key.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"

#include <cstring>

namespace logicalaccess
{
	SAMAV2ISO7816Commands::SAMAV2ISO7816Commands()
	{
	}

	SAMAV2ISO7816Commands::~SAMAV2ISO7816Commands()
	{
	}
}
