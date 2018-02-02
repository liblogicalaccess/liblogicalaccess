/**
 * \file test_appIDs_brutForce_class.cpp
 * \author Leo Jullerot
 * \brief
 */

#include "test_appIDs_brutForce_class.h"

AppIDsBrutForce::AppIDsBrutForce(std::shared_ptr<logicalaccess::DESFireISO7816Commands> &command,
                                 unsigned int idApp) : cmd(command), appID(idApp)
{
    cmd->selectApplication(appID);

    std::shared_ptr<logicalaccess::DESFireKey> doldkey(new logicalaccess::DESFireKey());

    doldkey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    doldkey->setKeyType(logicalaccess::DF_KEY_AES);

        try
        {
            cmd->authenticate(15, doldkey);
        }
        catch (logicalaccess::CardException &ex) {
            if (ex.error_code() == logicalaccess::CardException::WRONG_P1_P2)
            {
                keyType = logicalaccess::DF_KEY_AES;
            }
            else if (ex.error_code() == logicalaccess::CardException::SECURITY_STATUS)
            {
                keyType = logicalaccess::DF_KEY_DES;
        }
    }
}

void AppIDsBrutForce::brutForceFileIDs()
{
    cmd->selectApplication(appID);
    for (unsigned char i = 0; i < 32; i++) {
        try {
            cmd->getFileSettings(i);
            fileIDs.push_back(i);
        }
        catch (logicalaccess::CardException &ex) {}
    }
}

void AppIDsBrutForce::brutForceKeyNo()
{
    cmd->selectApplication(appID);
    std::shared_ptr<logicalaccess::DESFireKey> key(new logicalaccess::DESFireKey());
    key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    key->setKeyType(keyType);

    if (keyType == logicalaccess::DF_KEY_AES)
    {
        if (appID == 0)
        {
            keysNo.push_back(0);
        }
        else {
            for (unsigned int &&i = 0; i < 14; i++) {
                try {
                    cmd->authenticate(static_cast<unsigned char>(i), key);
                    keysNo.push_back(i);
                }
                catch (logicalaccess::CardException &ex) {
                    if (ex.error_code() == logicalaccess::CardException::SECURITY_STATUS) {
                        keysNo.push_back(i);
                    }
                }
            }
        }
    }
    else if (keyType == logicalaccess::DF_KEY_DES)
    {
        for (unsigned char i = 0; i < 14; i++)
        {
            try
            {
                cmd->authenticate(i, key);
                keysNo.push_back(i);
            }
            catch (logicalaccess::CardException &ex)
            {
                if (!(ex.error_code() == logicalaccess::CardException::KEY_NUMBER_INVALID))
                {
                    keysNo.push_back(i);
                }
            }
        }
    }
}

std::vector<unsigned int> AppIDsBrutForce::getFilesIDs()
{
    return fileIDs;
}

int AppIDsBrutForce::getAppID()
{
    return appID;
}

std::vector<unsigned int> AppIDsBrutForce::getKeysNo()
{
    return keysNo;
}
std::string AppIDsBrutForce::getKeyType() {
    if (keyType == logicalaccess::DF_KEY_AES)
    {
        return "AES";
    }
    else if (keyType == logicalaccess::DF_KEY_DES)
    {
        return "DES";
    }
    return "Not defined";
}