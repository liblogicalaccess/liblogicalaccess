/**
 * \file mifareultralightccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C commands.
 */

#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightccommands.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightcchip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
std::shared_ptr<MifareUltralightCChip>
MifareUltralightCCommands::getMifareUltralightCChip()
{
    return std::dynamic_pointer_cast<MifareUltralightCChip>(getMifareUltralightChip());
}

void MifareUltralightCCommands::changeKey(std::shared_ptr<TripleDESKey> key)
{
    if (key && key->getLength() == 16)
    {
        ByteVector buf;
        for (int i = 15; i >= 0; --i)
        {
            buf.push_back(key->getData()[i]);
        }
        writePage(0x2C, ByteVector(buf.begin() + 8, buf.begin() + 12));
        writePage(0x2D, ByteVector(buf.begin() + 12, buf.begin() + 16));
        writePage(0x2E, ByteVector(buf.begin(), buf.begin() + 4));
        writePage(0x2F, ByteVector(buf.begin() + 4, buf.begin() + 8));
    }
}

void MifareUltralightCCommands::lockPage(int page)
{
    if (page < 16)
    {
        // MifareUltralightCommands::lockPage(page);
        ByteVector lockbits(4, 0x00);

        if (page >= 3 && page <= 7)
        {
            lockbits[2] |= static_cast<unsigned char>(1 << page);
        }
        else if (page >= 8 && page <= 15)
        {
            lockbits[3] |= static_cast<unsigned char>(1 << (page - 8));
        }

        writePage(2, lockbits);
    }
    else
    {
        ByteVector lockbits(4, 0x00);

        if (page >= 44 && page <= 47)
        {
            lockbits[1] |= (1 << 7);
        }
        else if (page == 43)
        {
            lockbits[1] |= (1 << 6);
        }
        else if (page == 42)
        {
            lockbits[1] |= (1 << 5);
        }
        else if (page == 41)
        {
            lockbits[1] |= (1 << 4);
        }
        else if (page >= 36 && page <= 39)
        {
            lockbits[0] |= (1 << 7);
        }
        else if (page >= 32 && page <= 35)
        {
            lockbits[0] |= (1 << 6);
        }
        else if (page >= 28 && page <= 31)
        {
            lockbits[0] |= (1 << 5);
        }
        else if (page >= 24 && page <= 27)
        {
            lockbits[0] |= (1 << 3);
        }
        else if (page >= 20 && page <= 23)
        {
            lockbits[0] |= (1 << 2);
        }
        else if (page >= 16 && page <= 19)
        {
            lockbits[0] |= (1 << 1);
        }

        writePage(0x28, lockbits);
    }
}

void MifareUltralightCCommands::authenticate(std::shared_ptr<AccessInfo> aiToUse)
{
    if (aiToUse)
    {
        std::shared_ptr<MifareUltralightCAccessInfo> mAi =
            std::dynamic_pointer_cast<MifareUltralightCAccessInfo>(aiToUse);
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument,
                                  "aiToUse must be a MifareUltralightCAccessInfo.");

        if (mAi->key)
        {
            authenticate(mAi->key);
        }
    }
}

void MifareUltralightCCommands::changeKey(std::shared_ptr<AccessInfo> aiToWrite)
{
    if (aiToWrite)
    {
        std::shared_ptr<MifareUltralightCAccessInfo> mAi =
            std::dynamic_pointer_cast<MifareUltralightCAccessInfo>(aiToWrite);
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument,
                                  "aiToWrite must be a MifareUltralightCAccessInfo.");

        if (mAi->key)
        {
            changeKey(mAi->key);
        }
    }
}
}