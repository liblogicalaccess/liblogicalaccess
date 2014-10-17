/**
 * \file keystorage.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Key storage base class.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"

namespace logicalaccess
{
    std::shared_ptr<KeyDiversification> KeyDiversification::getKeyDiversificationFromType(std::string kdiv)
    {
        std::shared_ptr<KeyDiversification> ret = LibraryManager::getInstance()->getKeyDiversification(kdiv);

        if (!ret)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Key Diversification Type not found on plugins.");
        }
        return ret;
    }
}