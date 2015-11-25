/**
 * \file keystorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Key storage base class.
 */

#include "logicalaccess/cards/IKSStorage.hpp"
#include "logicalaccess/cards/computermemorykeystorage.hpp"
#include "logicalaccess/cards/readermemorykeystorage.hpp"
#include "logicalaccess/cards/samkeystorage.hpp"

namespace logicalaccess
{
std::shared_ptr<KeyStorage> KeyStorage::getKeyStorageFromType(KeyStorageType kst)
{
    std::shared_ptr<KeyStorage> ret;

    switch (kst)
    {
    case KST_COMPUTER_MEMORY:
        ret.reset(new ComputerMemoryKeyStorage());
        break;

    case KST_READER_MEMORY:
        ret.reset(new ReaderMemoryKeyStorage());
        break;

    case KST_SAM:
        ret.reset(new SAMKeyStorage());
        break;

    case KST_SERVER:
        ret.reset(new IKSStorage());
        break;
    }

    return ret;
}
}