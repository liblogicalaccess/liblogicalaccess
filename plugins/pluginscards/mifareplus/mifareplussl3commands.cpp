/**
 * \file mifareplussl3commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#include <logicalaccess/logs.hpp>
#include <assert.h>
#include <logicalaccess/crypto/cmac.hpp>
#include "mifareplussl3commands.hpp"
#include "mifarepluschip.hpp"
#include "MifarePlusSL0Commands.hpp"
#include "MifarePlusAESAuth.hpp"
#include "MifarePlusSL3Auth.hpp"

namespace logicalaccess
{

    bool MifarePlusSL3Commands_NEW::authenticate(int sector, std::shared_ptr<AES128Key> key,
                                                 MifareKeyType type)
    {
        auth_.reset(new MifarePlusSL3Auth(getReaderCardAdapter()));
        return auth_->firstAuthenticate(sector, key, type);
    }

    ByteVector MifarePlusSL3Commands_NEW::readBinaryPlain(
            unsigned char blockno,
            size_t len)
    {
        assert(len / 16 < 255);
        uint8_t nb_block_to_read = static_cast<uint8_t>(len / 16);
/*
        ByteVector mac = {1, 2, 3, 4, 5, 6, 7 , 8};
        ByteVector cmd{0x34, blockno, 0x00, nb_block_to_read};
        cmd.insert(cmd.end(), mac.begin(), mac.end());*/
        assert(auth_);

        ByteVector data(16, 'a');
        data = {0x32, 0x14, 0xA5, 0xF4, 0xDE, 0x18, 0xAE, 0xC8, 0xDA, 0x6F,
                0x50, 0x33, 0x32, 0xB7, 0x10, 0xD7};
       // data = {0x02, 0xBB, 0x2A, 0x18, 0xBA, 0x7C, 0x4B, 0x54, 0xFC, 0x9F,
//                0x8C, 0xAE, 0x37, 0x7F, 0x5C, 0x1A};
        //data = auth_->cipherWriteData(data);

        LOG(DEBUGS) << "Data encryp: " << data;
        ByteVector cmd = {0xA3, blockno, 0x00};
        cmd.insert(cmd.end(), data.begin(), data.end());

        auto mac = auth_->computeWriteMac(0xA3, blockno, data);
        //auto mac = GetMacOnCommand(cmd, auth_->trans_id_, auth_->deriveKMac());
        LOG(DEBUGS) << "Mac = " << mac;
        cmd.insert(cmd.end(), mac.begin(), mac.end());

        auto ret = getReaderCardAdapter()->sendCommand(cmd);
        LOG(DEBUGS) << "Ret from plain read" << ret;
        return ret;
    }

    void MifarePlusSL3Commands_NEW::resetAuth()
    {
        LOG(ERRORS) << "HOHO RESETTING AUTH";
        using ByteVector = std::vector<unsigned char>;
        ByteVector command {0x78};
        ByteVector ret;

        ret = getReaderCardAdapter()->sendCommand(command);
        LOG(DEBUGS) << "Ret from ResetAuth: " << ret;
    }
}
