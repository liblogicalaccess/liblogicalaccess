/**
 * \file mifareplussl3profile.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include "mifareplussl3profile.hpp"

namespace logicalaccess
{
    MifarePlusSL3Profile::MifarePlusSL3Profile()
        : MifarePlusProfile()
    {
        d_sectorKeys = new std::shared_ptr<MifarePlusKey>[(getNbSectors() + 1) * 2];
        for (unsigned int i = 0; i <= getNbSectors(); i++)
        {
            d_sectorKeys[i * 2] = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
            d_sectorKeys[i * 2 + 1] = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        }
        d_originalityKey = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        d_masterCardKey = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        d_configurationKey = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

        clearKeys();
    }

    MifarePlusSL3Profile::~MifarePlusSL3Profile()
    {
        delete[] d_sectorKeys;
    }

    void MifarePlusSL3Profile::clearKeys()
    {
        for (unsigned int i = 0; i <= getNbSectors(); i++)
        {
            d_sectorKeys[i * 2].reset();
            d_sectorKeys[i * 2 + 1].reset();
        }
        d_originalityKey.reset();
        d_masterCardKey.reset();
        d_configurationKey.reset();
    }

    std::shared_ptr<MifarePlusKey> MifarePlusSL3Profile::getKey(int index, MifarePlusKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
        {
            return std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        }

        switch (keytype)
        {
        case KT_KEY_AES_A:
        {
            return d_sectorKeys[index * 2];
        }
        case KT_KEY_AES_B:
        {
            return d_sectorKeys[index * 2 + 1];
        }
        case KT_KEY_ORIGINALITY:
        {
            return d_originalityKey;
        }
        case KT_KEY_MASTERCARD:
        {
            return d_masterCardKey;
        }
        case KT_KEY_CONFIGURATION:
        {
            return d_configurationKey;
        }
        default:
        {
            return std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        }
        }
    }

    void MifarePlusSL3Profile::setKey(int index, MifarePlusKeyType keytype, std::shared_ptr<MifarePlusKey> /*key*/)
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        /*		switch (keytype)
        {
        case KT_KEY_AES_A:
        {
        d_sectorKeys[index * 2] = key;

        break;
        }
        case KT_KEY_AES_B:
        {
        d_sectorKeys[index * 2 + 1] = key;

        break;
        }
        case KT_KEY_ORIGINALITY:
        {
        d_originalityKey = key;

        break;
        }
        case KT_KEY_MASTERCARD:
        {
        d_masterCardKey = key;

        break;
        }
        case KT_KEY_CONFIGURATION:
        {
        d_configurationKey = key;

        break;
        }
        }*/
    }

    bool MifarePlusSL3Profile::getKeyUsage(int index, MifarePlusKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        switch (keytype)
        {
        case KT_KEY_AES_A:
        {
            return bool(d_sectorKeys[index * 2]);
        }
        case KT_KEY_AES_B:
        {
            return bool(d_sectorKeys[index * 2 + 1]);
        }
        case KT_KEY_ORIGINALITY:
        {
            return bool(d_originalityKey);
        }
        case KT_KEY_MASTERCARD:
        {
            return bool(d_masterCardKey);
        }
        case KT_KEY_CONFIGURATION:
        {
            return bool(d_configurationKey);
        }
        default:
        {
            return false;
        }
        }
    }

    void MifarePlusSL3Profile::setKeyUsage(int index, MifarePlusKeyType keytype, bool used)
    {
        if (index > static_cast<int>(getNbSectors()) && (keytype == KT_KEY_AES_A || keytype == KT_KEY_AES_B))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        /*std::shared_ptr<MifarePlusKey> key;

        if (used)
            key = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));

        switch (keytype)
        {
        case KT_KEY_AES_A:
        {
        d_sectorKeys[index * 2] = key;
        }
        case KT_KEY_AES_B:
        {
        d_sectorKeys[index * 2 + 1] = key;
        }
        case KT_KEY_ORIGINALITY:
        {
        d_originalityKey = key;
        }
        case KT_KEY_MASTERCARD:
        {
        d_masterCardKey = key;
        }
        case KT_KEY_CONFIGURATION:
        {
        d_configurationKey = key;
        }
        }*/
    }

    void MifarePlusSL3Profile::setDefaultKeysAt(int index)
    {
        d_sectorKeys[index * 2].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));

        d_sectorKeys[index * 2 + 1].reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
    }

    void MifarePlusSL3Profile::setDefaultKeys()
    {
        for (unsigned int i = 0; i < getNbSectors(); i++)
        {
            setDefaultKeysAt(i);
        }
        d_originalityKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
        d_masterCardKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
        d_configurationKey.reset(new MifarePlusKey(MIFARE_PLUS_DEFAULT_AESKEY, MIFARE_PLUS_AES_KEY_SIZE, MIFARE_PLUS_AES_KEY_SIZE));
    }

    void MifarePlusSL3Profile::setKeyAt(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> AccessInfo)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

        std::shared_ptr<MifarePlusLocation> dfLocation = std::dynamic_pointer_cast<MifarePlusLocation>(location);
        std::shared_ptr<MifarePlusAccessInfo> dfAi = std::dynamic_pointer_cast<MifarePlusAccessInfo>(AccessInfo);

        EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(dfAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

        if (!dfAi->keyA->isEmpty())
        {
            setKey(dfLocation->sector, KT_KEY_AES_A, dfAi->keyA);
        }
        if (!dfAi->keyB->isEmpty())
        {
            setKey(dfLocation->sector, KT_KEY_AES_B, dfAi->keyB);
        }
        if (!dfAi->keyOriginality->isEmpty())
        {
            setKey(0, KT_KEY_ORIGINALITY, dfAi->keyOriginality);
        }
        if (!dfAi->keyMastercard->isEmpty())
        {
            setKey(0, KT_KEY_MASTERCARD, dfAi->keyMastercard);
        }
        if (!dfAi->keyConfiguration->isEmpty())
        {
            setKey(0, KT_KEY_CONFIGURATION, dfAi->keyConfiguration);
        }
    }

    std::shared_ptr<AccessInfo> MifarePlusSL3Profile::createAccessInfo() const
    {
        std::shared_ptr<MifarePlusAccessInfo> ret;
        ret.reset(new MifarePlusAccessInfo(MIFARE_PLUS_AES_KEY_SIZE));
        return ret;
    }
}
