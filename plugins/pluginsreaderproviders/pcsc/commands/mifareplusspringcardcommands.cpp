/**
 * \file mifareplusspringcardcommands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare Plus Springcard commands.
 */

#include "mifareplusspringcardcommands.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace logicalaccess
{
    MifarePlusSpringCardCommands::MifarePlusSpringCardCommands()
        : MifarePlusCommands()
    {
    }

    MifarePlusSpringCardCommands::~MifarePlusSpringCardCommands()
    {
    }

    bool MifarePlusSpringCardCommands::GenericAESAuthentication(unsigned short keyBNr, boost::shared_ptr<MifarePlusKey> key, bool t_cl, bool first)
    {
        std::vector<unsigned char> command;
        std::vector<unsigned char> res;
        std::vector<unsigned char> RndB;
        std::vector<unsigned char> RndA;
        std::vector<unsigned char> tmp;

        // init authentication and give the Key Block Number to the card
        command.resize(3);
        command[0] = 0x76;
        command[1] = static_cast<unsigned char>(keyBNr);
        command[2] = static_cast<unsigned char>(keyBNr >> 8);
        if (first)
        {
            command.resize(4);
            command[0] = 0x70;
            std::vector<unsigned char> PCDcap = GetCrypto()->d_PCDcap;
            command[3] = static_cast<unsigned char>(PCDcap.size());
            command.insert(command.end(), PCDcap.begin(), PCDcap.end());
        }

        res = AESSendCommand(command, t_cl, 1000);

        // check response and get RndB
        if (res.size() == 19)
        {
            res = std::vector<unsigned char>(res.begin() + 1, res.end() - 2);
            RndB = GetCrypto()->AESUncipher(res, key->getDataToBuffer(), false);
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The Key Block Number must be wrong or bad formatted, card didn't respond correctly");
        }

        // authentication command containing RndA and RndB with AES encryption
        RndA = GetRandKey(MIFARE_PLUS_AES_KEY_SIZE);
        tmp.insert(tmp.end(), RndA.begin(), RndA.end());
        std::vector<unsigned char> leftRot = LeftRotateSB(RndB);
        tmp.insert(tmp.end(), leftRot.begin(), leftRot.end());
        tmp = GetCrypto()->AESCipher(tmp, key->getDataToBuffer(), false);
        command.resize(1);
        command[0] = 0x72;
        command.insert(command.end(), tmp.begin(), tmp.end());

        res = AESSendCommand(command, t_cl, 1000);

        // check the response
        if ((!first && res.size() == 19) || (first && res.size() == 35))
        {
            res = std::vector<unsigned char>(res.begin() + 1, res.end() - 2);
            GetCrypto()->d_Wctr = 0x0000;
            GetCrypto()->d_Rctr = 0x0000;
            if (!first && LeftRotateSB(RndA) == GetCrypto()->AESUncipher(res, key->getDataToBuffer(), false))
                return true;
            else if (first)
            {
                res = GetCrypto()->AESUncipher(res, key->getDataToBuffer(), false);

                GetCrypto()->d_PCDcap = std::vector<unsigned char>(res.end() - 6, res.end());
                GetCrypto()->d_PICCcap = std::vector<unsigned char>(res.end() - 12, res.end() - 6);
                GetCrypto()->d_ti = std::vector<unsigned char>(res.begin(), res.begin() + 4);
                res = std::vector<unsigned char>(res.begin() + 4, res.end() - 12);
                if (LeftRotateSB(RndA) == res)
                {
                    //set the crypto informations
                    GetCrypto()->d_rndA = RndA;
                    GetCrypto()->d_rndB = RndB;
                    GetCrypto()->d_AESauthkey = key->getDataToBuffer();
                    GetCrypto()->calculateKmac();
                    GetCrypto()->calculateKenc();

                    return true;
                }
            }
        }
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Authentication failed, verify the AES key");

        return false;
    }

    std::vector<unsigned char> MifarePlusSpringCardCommands::AESSendCommand(const std::vector<unsigned char>& command, bool t_cl, long int /*timeout*/)
    {
        std::vector<unsigned char> result;

        if (t_cl)
            result = boost::dynamic_pointer_cast<PCSCReaderUnit>(getReaderCardAdapter()->getDataTransport()->getReaderUnit())->getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xFE, 0x00, 0x00, static_cast<unsigned char>(command.size()), command);
        else
            result = boost::dynamic_pointer_cast<PCSCReaderUnit>(getReaderCardAdapter()->getDataTransport()->getReaderUnit())->getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xFE, 0x01, 0x07, static_cast<unsigned char>(command.size()), command);

        // temp
        std::cout << "card responded : " << BufferHelper::getHex(result) << "(" << result.size() << " bytes)\n";
        //end
        CheckCardReturn(&result[0], result.size());

        return result;
    }

    void MifarePlusSpringCardCommands::CheckCardReturn(const unsigned char result[], size_t resultlen)
    {
        if (resultlen > 2 && result[0] != 0x90)
        {
            unsigned char code = result[0];

            switch (code)
            {
            case 0x06:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "Authentication error (reader application doesn't have the right key, access condition not fulfilled, ...)");
            }
                break;

            case 0x07:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "Too many read or write commands in the session or in the transaction");
            }
                break;

            case 0x08:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "Invalid MAC in command");
            }
                break;

            case 0x09:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "This address is not valid");
            }
                break;

            case 0x0A:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "This address does not exist");
            }
                break;

            case 0x0B:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "This command is not available at current card state");
            }
                break;

            case 0x0C:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "Invalid Length in command");
            }
                break;

            case 0x0F:
            {
                THROW_EXCEPTION_WITH_LOG(CardException, "Failure in the operation of the PICC");
            }
                break;
            }
        }
        else if (resultlen <= 2)
        {
            THROW_EXCEPTION_WITH_LOG(CardException, "Card didn't respond");
        }
    }

    bool MifarePlusSpringCardCommands::TurnOnTCL()
    {
        std::vector<unsigned char> activateT_CL;
        std::vector<unsigned char> returnT_CL;

        activateT_CL.resize(5);
        activateT_CL[0] = 0xFF;
        activateT_CL[1] = 0xFB;
        activateT_CL[2] = 0x20;
        activateT_CL[3] = 0x01;
        activateT_CL[4] = 0x00;

        returnT_CL = getReaderCardAdapter()->sendCommand(activateT_CL, 1000);

        if (returnT_CL.size() == 14)
            return true;
        return false;
    }

    bool MifarePlusSpringCardCommands::TurnOffTCL()
    {
        std::vector<unsigned char> desactivateT_CL;
        std::vector<unsigned char> returnT_CL;

        desactivateT_CL.resize(5);
        desactivateT_CL[0] = 0xFF;
        desactivateT_CL[1] = 0xFB;
        desactivateT_CL[2] = 0x20;
        desactivateT_CL[3] = 0x00;
        desactivateT_CL[4] = 0x00;

        returnT_CL = getReaderCardAdapter()->sendCommand(desactivateT_CL, 1000);

        if (returnT_CL.size() == 2)
            return true;
        return false;
    }

    bool MifarePlusSpringCardCommands::WakeUpCard()
    {
        std::vector<unsigned char> wake;
        std::vector<unsigned char> wakeReturn;

        wake.resize(5);
        wake[0] = 0xFF;
        wake[1] = 0xFE;
        wake[2] = 0x01;
        wake[3] = 0x08;
        wake[4] = 0x00;

        wakeReturn = getReaderCardAdapter()->sendCommand(wake, 1000);

        if (wakeReturn.size() > 2)
            return true;
        return false;
    }
}