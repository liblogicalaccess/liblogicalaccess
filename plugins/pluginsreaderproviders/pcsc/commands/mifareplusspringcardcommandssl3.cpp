/**
 * \file mifareplusspringcardcommandssl3.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare Plus SpringCard commands SL3.
 */

#include "mifareplusspringcardcommandssl3.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "../pcscreaderprovider.hpp"
#include "mifarepluschip.hpp"

namespace logicalaccess
{
    MifarePlusSpringCardCommandsSL3::MifarePlusSpringCardCommandsSL3()
        : MifarePlusSpringCardCommands()
    {
    }

    MifarePlusSpringCardCommandsSL3::~MifarePlusSpringCardCommandsSL3()
    {
    }

    bool MifarePlusSpringCardCommandsSL3::genericAuthenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype, bool first)
    {
        unsigned short KeyBlockNumber = 0x0000;
        switch (keytype)
        {
        case KT_KEY_AES_A:
            KeyBlockNumber = static_cast<unsigned short>((0x40 << 8) + sector * 2);
            break;
        case KT_KEY_AES_B:
            KeyBlockNumber = static_cast<unsigned short>((0x40 << 8) + sector * 2 + 1);
            break;
        case KT_KEY_ORIGINALITY:
            KeyBlockNumber = 0x8000;
            break;
        case KT_KEY_MASTERCARD:
            KeyBlockNumber = 0x9000;
            break;
        case KT_KEY_CONFIGURATION:
            KeyBlockNumber = 0x9001;
            break;
        case KT_KEY_CRYPTO1_A:
            break;
        case KT_KEY_CRYPTO1_B:
            break;
        case KT_KEY_SWITCHL2:
            break;
        case KT_KEY_SWITCHL3:
            break;
        case KT_KEY_AESSL1:
            break;
        }
        return (MifarePlusSpringCardCommands::GenericAESAuthentication(KeyBlockNumber, key, true, first));
    }

    bool MifarePlusSpringCardCommandsSL3::authenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
    {
        return (genericAuthenticate(sector, key, keytype, true));
    }

    bool MifarePlusSpringCardCommandsSL3::followingAuthenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype)
    {
        return (genericAuthenticate(sector, key, keytype, false));
    }

    void MifarePlusSpringCardCommandsSL3::resetAuthentication()
    {
        std::vector<unsigned char> data;
        std::vector<unsigned char> ret;

        data.resize(1);
        data[0] = 0x78;

        ret = AESSendCommand(data, true, 1000);
        if (ret.size() != 3)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Authentication reset failed\n");
    }

    std::vector<unsigned char> MifarePlusSpringCardCommandsSL3::readBinary(short blockno, char len, bool encrypted, bool macOnCommand, bool macOnResponse)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;

        command.resize(4);
        command[0] = findReadCommandCode(encrypted, macOnCommand, macOnResponse);
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);
        command[3] = static_cast<unsigned char>(len);

        if (macOnCommand)
        {
            std::vector<unsigned char> mac = getMAC(command);
            command.insert(command.end(), mac.begin(), mac.end());
        }

        res = AESSendCommand(command, true, 1000);
        res.resize(res.size() - 2);
        GetCrypto()->d_Rctr++;

        if (macOnResponse)
            res = checkMAC(res, command);
        res = std::vector<unsigned char>(res.begin() + 1, res.end());

        if (encrypted && GetCrypto()->d_Kenc.size() > 0)
            res = GetCrypto()->AESUncipher(res, GetCrypto()->d_Kenc, true);
        else if (encrypted)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");

        return res;
    }

    void MifarePlusSpringCardCommandsSL3::updateBinary(short blockno, bool encrypted, bool macOnResponse, const std::vector<unsigned char>& buf)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> data;

        if (buf.size() >= 256)
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad buffer parameter.");

        command.resize(3);
        command[0] = findWriteCommandCode(encrypted, macOnResponse);
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);

        data.insert(data.end(), buf.begin(), buf.end());
        if (encrypted && GetCrypto()->d_Kenc.size() > 0)
            data = GetCrypto()->AESCipher(data, GetCrypto()->d_Kenc, true);
        else if (encrypted)
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");
        command.insert(command.end(), data.begin(), data.end());

        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());

        res = AESSendCommand(command, true, 1000);
        res.resize(res.size() - 2);
        GetCrypto()->d_Wctr++;

        if (macOnResponse)
            res = checkMAC(res, command);
    }

    unsigned char MifarePlusSpringCardCommandsSL3::findReadCommandCode(bool encrypted, bool macOnCommand, bool macOnResponse)
    {
        if (encrypted)
        {
            if (macOnCommand)
            {
                if (macOnResponse)
                    return 0x31;
                else
                    return 0x30;
            }
            else
            {
                if (macOnResponse)
                    return 0x35;
                else
                    return 0x34;
            }
        }
        if (macOnCommand)
        {
            if (macOnResponse)
                return 0x33;
            else
                return 0x32;
        }
        if (macOnResponse)
            return 0x37;

        return 0x36;
    }

    unsigned char MifarePlusSpringCardCommandsSL3::findWriteCommandCode(bool encrypted, bool macOnResponse)
    {
        if (encrypted)
        {
            if (macOnResponse)
                return 0xA1;
            else
                return 0xA0;
        }
        if (macOnResponse)
            return 0xA3;
        return 0xA2;
    }

    std::vector<unsigned char> MifarePlusSpringCardCommandsSL3::getMAC(std::vector<unsigned char> command)
    {
        return (GetCrypto()->GetMacOnCommand(command));
    }

    std::vector<unsigned char> MifarePlusSpringCardCommandsSL3::checkMAC(std::vector<unsigned char> res, std::vector<unsigned char> command)
    {
        std::vector<unsigned char> mac;
        if (res.size() > MIFARE_PLUS_MAC_SIZE)
        {
            mac = std::vector<unsigned char>(res.end() - MIFARE_PLUS_MAC_SIZE, res.end());
        }

        if (mac != GetCrypto()->GetMacOnResponse(res, command))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "MAC on card response does not correspond to the command. This may result of a security problem\n");

        res.resize(res.size() - 8);
        return (res);
    }

    bool MifarePlusSpringCardCommandsSL3::increment(short blockno, bool macOnResponse, int value)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> data;

        //set command header
        command.resize(3);
        if (macOnResponse)
            command[0] = 0xB1;
        else
            command[0] = 0xB0;
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);
        //set command data
        data.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data[0] = static_cast<unsigned char>(value);
        data[1] = static_cast<unsigned char>(value >> 8);
        data[2] = static_cast<unsigned char>(value >> 16);
        data[3] = static_cast<unsigned char>(value >> 24);
        data[4] = 0x80;
        if (GetCrypto()->d_Kenc.size() > 0)
            data = GetCrypto()->AESCipher(data, GetCrypto()->d_Kenc, true);
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");
        command.insert(command.end(), data.begin(), data.end());
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;

        return false;
    }

    bool MifarePlusSpringCardCommandsSL3::decrement(short blockno, bool macOnResponse, int value)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> data;

        //set command header
        command.resize(3);
        if (macOnResponse)
            command[0] = 0xB3;
        else
            command[0] = 0xB2;
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);
        //set command data
        data.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data[0] = static_cast<unsigned char>(value);
        data[1] = static_cast<unsigned char>(value >> 8);
        data[2] = static_cast<unsigned char>(value >> 16);
        data[3] = static_cast<unsigned char>(value >> 24);
        data[4] = 0x80;
        if (GetCrypto()->d_Kenc.size() > 0)
            data = GetCrypto()->AESCipher(data, GetCrypto()->d_Kenc, true);
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");
        command.insert(command.end(), data.begin(), data.end());
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;
        return false;
    }

    bool MifarePlusSpringCardCommandsSL3::transfer(short blockno, bool macOnResponse)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;

        //set command header
        command.resize(3);
        if (macOnResponse)
            command[0] = 0xB5;
        else
            command[0] = 0xB4;
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;
        return false;
    }

    bool MifarePlusSpringCardCommandsSL3::incrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> data;

        //set command header
        command.resize(5);
        if (macOnResponse)
            command[0] = 0xB7;
        else
            command[0] = 0xB6;
        command[1] = static_cast<unsigned char>(sourceBlockno);
        command[2] = static_cast<unsigned char>(sourceBlockno >> 8);
        command[3] = static_cast<unsigned char>(destBlockno);
        command[4] = static_cast<unsigned char>(destBlockno >> 8);
        //set command data
        data.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data[0] = static_cast<unsigned char>(value);
        data[1] = static_cast<unsigned char>(value >> 8);
        data[2] = static_cast<unsigned char>(value >> 16);
        data[3] = static_cast<unsigned char>(value >> 24);
        data[4] = 0x80;
        if (GetCrypto()->d_Kenc.size() > 0)
            data = GetCrypto()->AESCipher(data, GetCrypto()->d_Kenc, true);
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");
        command.insert(command.end(), data.begin(), data.end());
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;

        return false;
    }

    bool MifarePlusSpringCardCommandsSL3::decrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> data;

        //set command header
        command.resize(5);
        if (macOnResponse)
            command[0] = 0xB9;
        else
            command[0] = 0xB8;
        command[1] = static_cast<unsigned char>(sourceBlockno);
        command[2] = static_cast<unsigned char>(sourceBlockno >> 8);
        command[3] = static_cast<unsigned char>(destBlockno);
        command[4] = static_cast<unsigned char>(destBlockno >> 8);
        //set command data
        data.resize(MIFARE_PLUS_BLOCK_SIZE, 0x00);
        data[0] = static_cast<unsigned char>(value);
        data[1] = static_cast<unsigned char>(value >> 8);
        data[2] = static_cast<unsigned char>(value >> 16);
        data[3] = static_cast<unsigned char>(value >> 24);
        data[4] = 0x80;
        if (GetCrypto()->d_Kenc.size() > 0)
            data = GetCrypto()->AESCipher(data, GetCrypto()->d_Kenc, true);
        else
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Session encryption key is null, you must set it first. Maybe the authentication failed\n");
        command.insert(command.end(), data.begin(), data.end());
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;

        return false;
    }

    bool MifarePlusSpringCardCommandsSL3::restore(short blockno, bool macOnResponse)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;

        //set command header
        command.resize(3);
        if (macOnResponse)
            command[0] = 0xC3;
        else
            command[0] = 0xC2;
        command[1] = static_cast<unsigned char>(blockno);
        command[2] = static_cast<unsigned char>(blockno >> 8);
        //add MAC
        std::vector<unsigned char> mac = getMAC(command);
        command.insert(command.end(), mac.begin(), mac.end());
        //send
        res = AESSendCommand(command, true, 1000);
        GetCrypto()->d_Wctr++;
        res.resize(res.size() - 2);
        //check
        if (macOnResponse)
            res = checkMAC(res, command);

        if (res[0] == 0x90)
            return true;

        return false;
    }
}