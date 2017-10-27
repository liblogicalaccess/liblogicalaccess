/**
 * \file desfirecommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire commands.
 */

#include "desfirecommands.hpp"
#include "desfirechip.hpp"
#include <cstring>

namespace logicalaccess
{
DESFireCommands::DataFileSetting DESFireCommands::FileSetting::getDataFile() const
{
    return this->type.dataFile;
}

void DESFireCommands::FileSetting::setDataFile(
    const DESFireCommands::DataFileSetting &settings)
{
    this->type.dataFile = settings;
}

DESFireCommands::ValueFileSetting DESFireCommands::FileSetting::getValueFile() const
{
    return this->type.valueFile;
}

void DESFireCommands::FileSetting::setValueFile(
    const DESFireCommands::ValueFileSetting &settings)
{
    this->type.valueFile = settings;
}
DESFireCommands::RecordFileSetting DESFireCommands::FileSetting::getRecordFile() const
{
    return this->type.recordFile;
}

void DESFireCommands::FileSetting::setRecordFile(
    const DESFireCommands::RecordFileSetting &settings)
{
    this->type.recordFile = settings;
}

std::shared_ptr<DESFireChip> DESFireCommands::getDESFireChip() const
{
    return std::dynamic_pointer_cast<DESFireChip>(getChip());
}

void DESFireCommands::selectApplication(std::shared_ptr<DESFireLocation> location)
{
    selectApplication(location->aid);
}

void DESFireCommands::createApplication(std::shared_ptr<DESFireLocation> location,
                                        DESFireKeySettings settings,
                                        unsigned char maxNbKeys)
{
    createApplication(location->aid, settings, maxNbKeys);
}

void DESFireCommands::createStdDataFile(std::shared_ptr<DESFireLocation> location,
                                        const DESFireAccessRights &accessRights,
                                        unsigned int fileSize)
{
    createStdDataFile(location->file, location->securityLevel, accessRights, fileSize);
}

EncryptionMode DESFireCommands::getEncryptionMode(unsigned char fileno, bool isReadMode,
                                                  bool *needLoadKey)
{
    DESFireCommands::FileSetting fileSetting;
    memset(&fileSetting, 0x00, sizeof(fileSetting));

    fileSetting = getFileSettings(fileno);

    return getEncryptionMode(fileSetting, isReadMode, needLoadKey);
}

EncryptionMode
DESFireCommands::getEncryptionMode(const DESFireCommands::FileSetting &fileSetting,
                                   bool isReadMode, bool *needLoadKey)
{
    EncryptionMode encMode    = CM_ENCRYPT;
    unsigned char accessRight = (isReadMode) ? (fileSetting.accessRights[1] >> 4)
                                             : (fileSetting.accessRights[1] & 0xF);
    unsigned char rwAccessRight = (fileSetting.accessRights[0] >> 4);
    if (accessRight == 0xE || rwAccessRight == 0xE)
    {
        // If accessRight or rwAccessRight is not set to Free then the communication could
        // be MACed instead of PLAIN
        // Ignored here.
        encMode = CM_PLAIN;
        if (needLoadKey != nullptr)
        {
            *needLoadKey = false;
        }
    }
    else
    {
        switch ((fileSetting.comSett & 0x3))
        {
        case 0:
        case 2: encMode = CM_PLAIN; break;

        case 1: encMode = CM_MAC; break;

        case 3: encMode = CM_ENCRYPT; break;
        default:;
        }
    }

    return encMode;
}

unsigned int DESFireCommands::getFileLength(unsigned char fileno)
{
    unsigned int fileLength = 0x00;

    DESFireCommands::FileSetting fileSetting;
    memset(&fileSetting, 0x00, sizeof(fileSetting));
    fileSetting = getFileSettings(fileno);

    switch (fileSetting.fileType)
    {
    case 0:
    {
        memcpy(&fileLength, fileSetting.type.dataFile.fileSize,
               sizeof(fileSetting.type.dataFile.fileSize));
    }
    break;
    default:;
    }

    return fileLength;
}
}