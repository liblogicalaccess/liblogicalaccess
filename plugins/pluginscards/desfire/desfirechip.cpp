/**
 * \file desfirechip.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire chip.
 */

#include "desfirechip.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "desfirestoragecardservice.hpp"

#include <cstring>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
    DESFireChip::DESFireChip(std::string ct) :
        Chip(ct),
        has_real_uid_(true)
    {
		d_crypto.reset(new DESFireCrypto());
    }

    DESFireChip::DESFireChip() :
        Chip(CHIP_DESFIRE),
        has_real_uid_(true)
    {
		d_crypto.reset(new DESFireCrypto());
    }

    DESFireChip::~DESFireChip()
    {
    }

    std::shared_ptr<LocationNode> DESFireChip::getRootLocationNode()
    {
        std::shared_ptr<LocationNode> rootNode;
        rootNode.reset(new LocationNode());

        rootNode->setName("Mifare DESFire");
        rootNode->setHasProperties(true);

        std::shared_ptr<DESFireLocation> rootLocation = std::dynamic_pointer_cast<DESFireLocation>(createLocation());
        rootLocation->aid = (unsigned int)-1;
        rootLocation->file = (unsigned char)-1;
        rootNode->setLocation(rootLocation);

        if (getCommands())
        {
            getDESFireCommands()->selectApplication(0);

            // Try authentication.
            try
            {
                getDESFireCommands()->authenticate(0);
            }
            catch (CardException&)
            {
            }

            std::vector<unsigned int> aids = getDESFireCommands()->getApplicationIDs();

            for (std::vector<unsigned int>::const_iterator aid = aids.cbegin(); aid != aids.cend(); ++aid)
            {
                char tmpName[32];
                std::shared_ptr<LocationNode> aidNode;
                aidNode.reset(new LocationNode());
                sprintf(tmpName, "Application ID %u", *aid);
                aidNode->setName(tmpName);

                std::shared_ptr<DESFireLocation> aidLocation = std::dynamic_pointer_cast<DESFireLocation>(createLocation());
                aidLocation->aid = *aid;
                aidLocation->file = static_cast<unsigned char>(-1);
                aidNode->setLocation(aidLocation);

                getDESFireCommands()->selectApplication(*aid);
                // Try authentication.
                try
                {
                    getDESFireCommands()->authenticate(0);
                }
                catch (CardException&)
                {
                }

                try
                {
                    std::vector<unsigned char> files = getDESFireCommands()->getFileIDs();

                    for (std::vector<unsigned char>::const_iterator file = files.cbegin(); file != files.cend(); ++file)
                    {
                        std::shared_ptr<LocationNode> fileNode;
                        fileNode.reset(new LocationNode());
                        sprintf(tmpName, "File %d", *file);
                        fileNode->setName(tmpName);

                        std::shared_ptr<DESFireLocation> location = getApplicationLocation();
                        location->aid = *aid;
                        location->file = *file;
                        location->byte_ = 0;

                        try
                        {
                            DESFireCommands::FileSetting settings;
                            getDESFireCommands()->getFileSettings(*file, settings);

							// We assume the action when using location node enumeration will be for read access
							location->securityLevel = getDESFireCommands()->getEncryptionMode(settings, true);
                            switch (settings.fileType)
                            {
                            case 0:
                            {
                                size_t fileSize = 0;
                                memcpy(&fileSize, settings.type.dataFile.fileSize, sizeof(settings.type.dataFile.fileSize));
                                fileNode->setLength(fileSize);
                            }
                                break;

                            case 1:
                            {
                                //TODO: Write something here ?
                            }
                                break;

                            case 2:
                            {
                                size_t recordSize = 0;
                                memcpy(&recordSize, settings.type.recordFile.recordSize, sizeof(settings.type.recordFile.recordSize));
                                fileNode->setLength(recordSize);
                            }
                                break;
                            }
                        }
                        catch (std::exception&)
                        {
                            fileNode->setLength(0);
                        }

                        fileNode->setNeedAuthentication(true);
                        fileNode->setHasProperties(true);
                        fileNode->setLocation(location);
                        fileNode->setParent(aidNode);
                        aidNode->getChildrens().push_back(fileNode);
                    }
                }
                catch (std::exception&)
                {
                }

                aidNode->setHasProperties(true);
                aidNode->setParent(rootNode);
                rootNode->getChildrens().push_back(aidNode);
            }
        }

        return rootNode;
    }

    std::shared_ptr<DESFireLocation> DESFireChip::getApplicationLocation()
    {
        std::shared_ptr<DESFireLocation> location(new DESFireLocation());

        return location;
    }

    std::shared_ptr<CardService> DESFireChip::getService(CardServiceType serviceType)
    {
        std::shared_ptr<CardService> service;

        switch (serviceType)
        {
        case CST_ACCESS_CONTROL:
        {
            service = LibraryManager::getInstance()
                ->getCardService(shared_from_this(), CardServiceType::CST_ACCESS_CONTROL);
            if (!service)
                service.reset(new AccessControlCardService(shared_from_this()));
        }
            break;
        case CST_STORAGE:
        {
            service.reset(new DESFireStorageCardService(shared_from_this()));
        }
            break;
        default:
            break;
        }

        if (!service)
        {
            service = Chip::getService(serviceType);
        }

        return service;
    }

	std::shared_ptr<AccessInfo> DESFireChip::createAccessInfo() const
	{
		std::shared_ptr<DESFireAccessInfo> ret;
		ret.reset(new DESFireAccessInfo());
		return ret;
	}

	std::shared_ptr<Location> DESFireChip::createLocation() const
	{
		std::shared_ptr<DESFireLocation> ret;
		ret.reset(new DESFireLocation());
		return ret;
	}
}