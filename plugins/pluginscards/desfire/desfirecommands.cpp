/**
 * \file desfirecommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#include "desfirecommands.hpp"
#include "desfirechip.hpp"

namespace logicalaccess
{
    boost::shared_ptr<DESFireChip> DESFireCommands::getDESFireChip() const
    {
        return boost::dynamic_pointer_cast<DESFireChip>(getChip());
    }

    void DESFireCommands::selectApplication(boost::shared_ptr<DESFireLocation> location)
    {
        selectApplication(location->aid);
    }

    void DESFireCommands::createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, unsigned char maxNbKeys)
    {
        createApplication(location->aid, settings, maxNbKeys);
    }

    void DESFireCommands::createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, unsigned int fileSize)
    {
        createStdDataFile(location->file, location->securityLevel, accessRights, fileSize);
    }

    EncryptionMode DESFireCommands::getEncryptionMode(boost::shared_ptr<AccessInfo> aiToUse, unsigned char fileno, bool isReadMode, bool* needLoadKey = NULL)
    {
        EncryptionMode encMode = CM_ENCRYPT;
        DESFireCommands::FileSetting fileSetting;
        memset(&fileSetting, 0x00, sizeof(fileSetting));

        getFileSettings(fileno, fileSetting);

        unsigned char accessRight = (isReadMode) ? (fileSetting.accessRights[0] >> 4) : (fileSetting.accessRights[0] & 0xF);
        if (accessRight == 0xE)
        {
            accessRight = (fileSetting.accessRights[1] >> 4);
            if (accessRight == 0xE)
            {
                encMode = CM_PLAIN;
                if (needLoadKey != NULL)
                {
                    *needLoadKey = false;
                }
            }
            else
            {
                if (aiToUse != NULL)
                {
                    encMode = CM_MAC;
                }
                else
                {
                    encMode = CM_PLAIN;
                }
            }
        }
        else
        {
            switch ((fileSetting.comSett & 0x3))
            {
            case 0:
            case 2:
                encMode = CM_PLAIN;
                break;

            case 1:
                encMode = CM_MAC;
                break;

            case 3:
                encMode = CM_ENCRYPT;
                break;
            }
        }

        return encMode;
    }

    unsigned int DESFireCommands::getFileLength(unsigned char fileno)
    {
        unsigned int fileLength = 0x00;

        DESFireCommands::FileSetting fileSetting;
        memset(&fileSetting, 0x00, sizeof(fileSetting));
        getFileSettings(fileno, fileSetting);

        switch (fileSetting.fileType)
        {
        case 0:
        {
            memcpy(&fileLength, fileSetting.type.dataFile.fileSize, sizeof(fileSetting.type.dataFile.fileSize));
        }
            break;
        }

        return fileLength;
    }
}