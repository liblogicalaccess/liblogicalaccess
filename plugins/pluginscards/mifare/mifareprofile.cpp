/**
 * \file mifareprofile.cpp
 * \author Julien K. <julien-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Mifare card profiles.
 */

#include <logicalaccess/logs.hpp>
#include "mifareprofile.hpp"

namespace logicalaccess
{
    MifareProfile::MifareProfile()
        : Profile()
    {
        d_key = new std::shared_ptr<MifareKey>[(getNbSectors() + 1) * 2];

        clearKeys();
    }

    MifareProfile::~MifareProfile()
    {
        delete[] d_key;
    }

    unsigned int MifareProfile::getNbSectors() const
    {
        return 40;
    }

    void MifareProfile::setDefaultKeys()
    {
        for (unsigned int i = 0; i < getNbSectors(); i++)
        {
            setDefaultKeysAt(i);
        }
    }

    void MifareProfile::setDefaultKeysAt(std::shared_ptr<Location> location)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");

        if (mLocation->sector != -1)
        {
            setDefaultKeysAt(mLocation->sector);
        }
    }

    void MifareProfile::clearKeys()
    {
        for (unsigned int i = 0; i <= getNbSectors(); i++)
        {
            d_key[i * 2].reset();
            d_key[i * 2 + 1].reset();
        }
    }

    std::shared_ptr<MifareKey> MifareProfile::getKey(int index, MifareKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()))
        {
            return std::shared_ptr<MifareKey>(new MifareKey());
        }

        switch (keytype)
        {
        case KT_KEY_A:
        {
            return d_key[index * 2];
        }
        case KT_KEY_B:
        {
            return d_key[index * 2 + 1];
        }
        default:
        {
            return std::shared_ptr<MifareKey>(new MifareKey());
        }
        }
    }

    void MifareProfile::setKey(int index, MifareKeyType keytype, std::shared_ptr<MifareKey> key)
    {
        if (index > static_cast<int>(getNbSectors()))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        switch (keytype)
        {
        case KT_KEY_A:
        {
            d_key[index * 2] = key;

            break;
        }
        case KT_KEY_B:
        {
            d_key[index * 2 + 1] = key;

            break;
        }
        }
    }

    bool MifareProfile::getKeyUsage(int index, MifareKeyType keytype) const
    {
        if (index > static_cast<int>(getNbSectors()))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        switch (keytype)
        {
        case KT_KEY_A:
        {
            return bool(d_key[index * 2]);
        }
        case KT_KEY_B:
        {
            return bool(d_key[index * 2 + 1]);
        }
        default:
        {
            return false;
        }
        }
    }

    void MifareProfile::setKeyUsage(int index, MifareKeyType keytype, bool used)
    {
        if (index > static_cast<int>(getNbSectors()))
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Index is greater than max sector number.");
        }

        std::shared_ptr<MifareKey> key;
        if (used)
        {
            key.reset(new MifareKey());
        }

        switch (keytype)
        {
        case KT_KEY_A:
        {
            d_key[index * 2] = key;
            break;
        }
        case KT_KEY_B:
        {
            d_key[index * 2 + 1] = key;
            break;
        }
        }
    }

    void MifareProfile::setDefaultKeysAt(int index)
    {
        d_key[index * 2].reset(new MifareKey());

        d_key[index * 2 + 1].reset(new MifareKey());
    }

    void MifareProfile::setKeyAt(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> AccessInfo)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
        EXCEPTION_ASSERT_WITH_LOG(AccessInfo, std::invalid_argument, "AccessInfo cannot be null.");

        std::shared_ptr<MifareLocation> mLocation = std::dynamic_pointer_cast<MifareLocation>(location);
        std::shared_ptr<MifareAccessInfo> mAi = std::dynamic_pointer_cast<MifareAccessInfo>(AccessInfo);

        EXCEPTION_ASSERT_WITH_LOG(mLocation, std::invalid_argument, "location must be a MifareLocation.");
        EXCEPTION_ASSERT_WITH_LOG(mAi, std::invalid_argument, "AccessInfo must be a MifareAccessInfo.");

        if (!mAi->keyA->isEmpty())
        {
            setKey(mLocation->sector, KT_KEY_A, mAi->keyA);
        }

        if (!mAi->keyB->isEmpty())
        {
            setKey(mLocation->sector, KT_KEY_B, mAi->keyB);
        }
    }

    std::shared_ptr<AccessInfo> MifareProfile::createAccessInfo() const
    {
        std::shared_ptr<MifareAccessInfo> ret;
        ret.reset(new MifareAccessInfo());
        return ret;
    }

    std::shared_ptr<Location> MifareProfile::createLocation() const
    {
        std::shared_ptr<MifareLocation> ret;
        ret.reset(new MifareLocation());
        return ret;
    }

    FormatList MifareProfile::getSupportedFormatList()
    {
        return Profile::getHIDWiegandFormatList();
    }
}