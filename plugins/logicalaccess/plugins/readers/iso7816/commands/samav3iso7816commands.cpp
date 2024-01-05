/**
 * \file SAMAV3ISO7816Commands.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3ISO7816Commands commands.
 */


#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav3iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunitconfiguration.hpp>
#include <logicalaccess/plugins/cards/samav/samcrypto.hpp>
#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/samkucentry.hpp>
#include <openssl/rand.h>
#include <logicalaccess/plugins/crypto/symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_symmetric_key.hpp>
#include <logicalaccess/plugins/crypto/aes_initialization_vector.hpp>
#include <logicalaccess/plugins/crypto/aes_cipher.hpp>
#include <logicalaccess/plugins/crypto/cmac.hpp>

#include <cstring>

namespace logicalaccess
{
SAMAV3ISO7816Commands::SAMAV3ISO7816Commands()
    : SAMAV2ISO7816Commands(CMD_SAMAV3ISO7816)
{
    
}

SAMAV3ISO7816Commands::SAMAV3ISO7816Commands(std::string ct)
    : SAMAV2ISO7816Commands(ct)
{

}

SAMAV3ISO7816Commands::~SAMAV3ISO7816Commands()
{
}

ByteVector SAMAV3ISO7816Commands::encipherKeyEntry(unsigned char keyno,
                                             unsigned char targetKeyno,
                                             unsigned short changeCounter,
                                             unsigned char channel,
                                             const ByteVector& targetSamUid,
                                             const ByteVector& divInput)
{
    unsigned char p2 = 0x00;
    ByteVector data;
    data.push_back(static_cast<unsigned char>(0x80 & channel));
    data.push_back(targetKeyno);
    data.push_back(static_cast<unsigned char>(0xff & (changeCounter >> 8)));
    data.push_back(static_cast<unsigned char>(0xff & changeCounter));
    if (targetSamUid.size() > 0)
    {
        p2 |= 0x01;
        data.insert(data.end(), targetSamUid.begin(), targetSamUid.end());
    }
    if (divInput.size() > 0)
    {
        p2 |= 0x02;
        data.insert(data.end(), divInput.begin(), divInput.end());
    }
    auto result = getISO7816ReaderCardAdapter()->sendAPDUCommand(d_cla, 0xE1, keyno, p2, static_cast<unsigned char>(data.size()), data, 0x00);
	return result.getData();
}
}