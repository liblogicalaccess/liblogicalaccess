/**
 * \file DESFireChip.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire chip.
 */

#include "DESFireChip.h"
#include "DESFireProfile.h"

#include <cstring>

namespace LOGICALACCESS
{
	DESFireChip::DESFireChip(std::string ct) : 
		Chip(ct)
	{
		
	}

	DESFireChip::DESFireChip() : 
		Chip("DESFire")
	{
		d_profile.reset(new DESFireProfile());
	}

	DESFireChip::~DESFireChip()
	{

	}

	boost::shared_ptr<LocationNode> DESFireChip::getRootLocationNode()
	{
		boost::shared_ptr<LocationNode> rootNode;
		rootNode.reset(new LocationNode());
		char tmpName[255];

		rootNode->setName("Mifare DESFire");
		rootNode->setHasProperties(true);

		boost::shared_ptr<DESFireLocation> rootLocation = boost::dynamic_pointer_cast<DESFireLocation>(getProfile()->createLocation());
		rootLocation->aid = (unsigned int)-1;
		rootLocation->file = (unsigned int)-1;
		rootNode->setLocation(rootLocation);

		if (d_cardprovider)
		{
			getDESFireCardProvider()->getDESFireCommands()->selectApplication(0);

			// Try authentication.
			try
			{
				getDESFireCardProvider()->getDESFireCommands()->authenticate(0);
			}
			catch(CardException&)
			{

			}

			std::vector<int> aids = getDESFireCardProvider()->getDESFireCommands()->getApplicationIDs();

			for (std::vector<int>::const_iterator aid = aids.begin(); aid != aids.end(); aid++)
			{
				boost::shared_ptr<LocationNode> aidNode;
				aidNode.reset(new LocationNode());
				sprintf(tmpName, "Application ID %u", *aid);
				aidNode->setName(tmpName);

				boost::shared_ptr<DESFireLocation> aidLocation = boost::dynamic_pointer_cast<DESFireLocation>(getProfile()->createLocation());
				aidLocation->aid = *aid;
				aidLocation->file = static_cast<unsigned int>(-1);
				aidNode->setLocation(aidLocation);

				if (getDESFireCardProvider()->getDESFireCommands()->selectApplication(*aid))
				{
					// Try authentication.
					try
					{
						getDESFireCardProvider()->getDESFireCommands()->authenticate(0);
					}
					catch(CardException&)
					{

					}

					try
					{
						std::vector<int> files = getDESFireCardProvider()->getDESFireCommands()->getFileIDs();
					
						for (std::vector<int>::const_iterator file = files.begin(); file != files.end(); ++file)
						{
							boost::shared_ptr<LocationNode> fileNode;
							fileNode.reset(new LocationNode());
							sprintf(tmpName, "File %d", *file);
							fileNode->setName(tmpName);

							boost::shared_ptr<DESFireLocation> location = getApplicationLocation();
							location->aid = *aid;
							location->file = *file;
							location->byte = 0;							

							try
							{
								DESFireCommands::FileSetting settings;
								if (getDESFireCardProvider()->getDESFireCommands()->getFileSettings(*file, settings))
								{
									location->securityLevel = (EncryptionMode)settings.comSett;
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
								else
								{
									location->securityLevel = CM_ENCRYPT;
								}
							}
							catch(std::exception&)
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
					catch(std::exception&)
					{
					}
				}

				aidNode->setHasProperties(true);
				aidNode->setParent(rootNode);
				rootNode->getChildrens().push_back(aidNode);
			}
		}

		return rootNode;
	}

	boost::shared_ptr<DESFireLocation> DESFireChip::getApplicationLocation()
	{
		boost::shared_ptr<DESFireLocation> location(new DESFireLocation());		

		return location;
	}
}
