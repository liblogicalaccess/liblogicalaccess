/**
 * \file mifareultralightccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C commands.
 */

#include "mifareultralightccommands.hpp"

namespace logicalaccess
{
    void MifareUltralightCCommands::changeKey(boost::shared_ptr<TripleDESKey> key)
    {
        if (key)
        {
            for (unsigned int i = 0; i < key->getLength(); i += 4)
            {
                writePage(0x2C + (i / 4), key->getData() + i, 4);
            }
        }
    }

    void MifareUltralightCCommands::lockPage(int page)
    {
        if (page < 16)
        {
            MifareUltralightCommands::lockPage(page);
        }
        else
        {
            unsigned char lockbits[4];
            memset(lockbits, 0x00, sizeof(lockbits));

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

            writePage(0x28, lockbits, sizeof(lockbits));
        }
    }

    void MifareUltralightCCommands::authenticate(boost::shared_ptr<AccessInfo> aiToUse)
    {
        if (aiToUse)
        {
            boost::shared_ptr<MifareUltralightCAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightCAccessInfo>(aiToUse);
            EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "aiToUse must be a MifareUltralightCAccessInfo.");

            if (mAi->key)
            {
                authenticate(mAi->key);
            }
        }
    }

    void MifareUltralightCCommands::changeKey(boost::shared_ptr<AccessInfo> aiToWrite)
    {
        if (aiToWrite)
        {
            boost::shared_ptr<MifareUltralightCAccessInfo> mAi = boost::dynamic_pointer_cast<MifareUltralightCAccessInfo>(aiToWrite);
            EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "aiToWrite must be a MifareUltralightCAccessInfo.");

            if (mAi->key)
            {
                changeKey(mAi->key);
            }
        }
    }
}