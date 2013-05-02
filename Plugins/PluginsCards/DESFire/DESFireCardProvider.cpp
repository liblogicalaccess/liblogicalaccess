/**
 * \file DESFireCardProvider.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief DESFire card provider.
 */

#include "DESFireCardProvider.h"
#include "DESFireChip.h"
#include "logicalaccess/Services/AccessControl/AccessControlCardService.h"
#include "DESFireEV1Location.h"

namespace LOGICALACCESS
{
	bool DESFireCardProvider::erase()
	{
		return getDESFireCommands()->erase();
	}

	bool DESFireCardProvider::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		boost::shared_ptr<DESFireAccessInfo> dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(aiToUse);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");

		bool ret = false;

		// Format the card if MCK specified.
		if (!dfAiToUse->masterCardKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(0, 0, dfAiToUse->masterCardKey);
			ret = getDESFireCommands()->erase();
		}
		// Format the application if MAK specified.
		else if (!dfAiToUse->masterApplicationKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, 0, dfAiToUse->masterApplicationKey);

			if (!getDESFireCommands()->selectApplication(dfLocation->aid))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_SELECTAPPLICATION);
			}

			std::vector<int> files = getDESFireCommands()->getFileIDs();
			for (std::vector<int>::const_iterator file = files.begin(); (file != files.end()); file++)
			{
				getDESFireCommands()->deleteFile(*file);
			}

			if (!getDESFireCommands()->changeKey(0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00")))))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
			}

			ret = true;
		}
		// Otherwise format the file.
		else
		{
			if (!getDESFireCommands()->selectApplication(dfLocation->aid))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_SELECTAPPLICATION);
			}

			size_t fileLength = getFileLength(static_cast<unsigned char>(dfLocation->file));
			unsigned char* buf =  new unsigned char[fileLength];
			memset(buf, 0x00, fileLength);

			boost::shared_ptr<AccessInfo> ai;			
			try
			{
				ret = writeData(location, aiToUse, ai, buf, fileLength, CB_DEFAULT);
			}
			catch(std::exception&)
			{
				delete[] buf;
				throw;
			}

			delete[] buf;			
		}

		return ret;
	}	

	void DESFireCardProvider::selectApplication(boost::shared_ptr<DESFireLocation> location)
	{
		if (!getDESFireCommands()->selectApplication(location->aid))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, EXCEPTION_MSG_SELECTAPPLICATION);
		}
	}

	void DESFireCardProvider::createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, int maxNbKeys)
	{
		if (!getDESFireCommands()->createApplication(location->aid, settings, maxNbKeys))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't create the application.");
		}
	}

	void DESFireCardProvider::createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, int fileSize)
	{
		if (!getDESFireCommands()->createStdDataFile(location->file, location->securityLevel, accessRights, fileSize))
		{
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "Can't create the file.");
		}
	}

	bool DESFireCardProvider::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "data cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		boost::shared_ptr<DESFireAccessInfo> dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(aiToUse);
		boost::shared_ptr<DESFireAccessInfo> dfAiToWrite = boost::dynamic_pointer_cast<DESFireAccessInfo>(aiToWrite);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");

		if (aiToUse != NULL)
		{
			EXCEPTION_ASSERT_WITH_LOG(dfAiToUse, std::invalid_argument, "aiToUse must be a DESFireAccessInfo.");
		}
		else
		{
			dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}
		
		if (aiToWrite)
		{
			EXCEPTION_ASSERT_WITH_LOG(dfAiToWrite, std::invalid_argument, "aiToWrite must be a DESFireAccessInfo.");
		}
		
		getDESFireChip()->getProfile()->clearKeys();

		if (!dfAiToUse->masterCardKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(0, 0, dfAiToUse->masterCardKey);
		}
		else
		{
			getDESFireChip()->getDESFireProfile()->setKey(0, 0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))));
		}

		getDESFireCommands()->selectApplication(0);

		bool createArbo = true;
		try
		{
			getDESFireCommands()->authenticate(0);

			// Create application if doesn't exist
			std::vector<int> aids = getDESFireCommands()->getApplicationIDs();

			if (aiToWrite)
			{
				for (std::vector<int>::const_iterator aid = aids.begin(); (aid != aids.end()) && createArbo; aid++)
				{
					createArbo = (*aid != dfLocation->aid);
				}
			}
		}
		catch(std::exception&)
		{

		}

		try
		{
			if (createArbo && dfAiToWrite)
			{
				int maxNbKeys = 0;
				if (dfAiToWrite->readKeyno > dfAiToWrite->writeKeyno)
				{
					if (dfAiToWrite->readKeyno < 0xe)
					{
						maxNbKeys = dfAiToWrite->readKeyno + 1;
					}
				}
				
				if (maxNbKeys == 0)
				{
					if (dfAiToWrite->writeKeyno < 0xe)
					{
						maxNbKeys = dfAiToWrite->writeKeyno + 1;
					}
				}

				if (maxNbKeys == 0)
				{
					maxNbKeys = 1;
				}

				createApplication(dfLocation, KS_DEFAULT, maxNbKeys);
			}
		}
		catch(std::exception&)
		{
			createArbo = false;
		}

		selectApplication(dfLocation);
		DESFireKeyType cryptoMethod = DF_KEY_DES;
		if (boost::dynamic_pointer_cast<DESFireEV1Location>(dfLocation))
		{
			cryptoMethod = boost::dynamic_pointer_cast<DESFireEV1Location>(dfLocation)->cryptoMethod;
		}

		if (!dfAiToUse->masterApplicationKey->isEmpty())
		{
			dfAiToUse->masterApplicationKey->setKeyType(cryptoMethod);
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, 0, dfAiToUse->masterApplicationKey);
		}
		else
		{
			if (!createArbo)
			{
				getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, 0, getDESFireChip()->getDESFireProfile()->getDefaultKey(cryptoMethod));
			}
		}

		if (!dfAiToUse->writeKey->isEmpty() && dfAiToUse->writeKeyno != 0x00)
		{
			dfAiToUse->writeKey->setKeyType(cryptoMethod);
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, dfAiToUse->writeKeyno, dfAiToUse->writeKey);
		}

		if (!dfAiToUse->readKey->isEmpty() && dfAiToUse->readKeyno != 0x00)
		{
			dfAiToUse->readKey->setKeyType(cryptoMethod);
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, dfAiToUse->readKeyno, dfAiToUse->readKey);
		}

		DESFireKeySettings appKeySettings = KS_DEFAULT;
		unsigned int appMaxNbKeys = 3;

		bool needLoadKey = true;
		createArbo = true;
		try
		{
			getDESFireCommands()->authenticate(0);

			getDESFireCommands()->getKeySettings(appKeySettings, appMaxNbKeys);
			std::vector<int> files = getDESFireCommands()->getFileIDs();

			if (aiToWrite)
			{
				for (std::vector<int>::const_iterator file = files.begin(); (file != files.end()) && createArbo; file++)
				{
					createArbo = (*file != dfLocation->file);
				}
			}
		}
		catch(std::exception&)
		{
		}

		try
		{
			if (createArbo && dfAiToWrite)
			{		
				DESFireAccessRights rights;
				rights.readAccess = (TaskAccessRights)dfAiToWrite->readKeyno; // AR_KEY1
				rights.writeAccess = (TaskAccessRights)dfAiToWrite->writeKeyno; // AR_KEY2;
				rights.readAndWriteAccess = (TaskAccessRights)dfAiToWrite->writeKeyno; //AR_KEY2
				rights.changeAccess = (TaskAccessRights)dfAiToWrite->writeKeyno;

				dfAiToUse->writeKeyno = dfAiToWrite->writeKeyno;

				if (dfLocation->securityLevel == CM_UNKNOWN)
				{
					dfLocation->securityLevel = CM_ENCRYPT;
				}

				createStdDataFile(dfLocation, rights, static_cast<int>(dataLength) + dfLocation->byte);
				needLoadKey = false;
			}
		}
		catch(std::exception&)
		{
		}

		if (needLoadKey && !dfAiToUse->writeKey->isEmpty() && dfAiToUse->writeKeyno != 0)
		{
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, dfAiToUse->writeKeyno, dfAiToUse->writeKey);
		}
		
		needLoadKey = true;
		EncryptionMode encMode = dfLocation->securityLevel;

		if (encMode == CM_UNKNOWN)
		{
			encMode = getEncryptionMode(aiToUse, static_cast<unsigned char>(dfLocation->file), false, &needLoadKey);
		}

		if (needLoadKey)
		{
			if (!getDESFireCommands()->authenticate(dfAiToUse->writeKeyno))
			{	
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_AUTHENTICATE);
			}
		}

		ret = getDESFireCommands()->writeData(dfLocation->file,
			dfLocation->byte,
			dataLength,
			data,
			encMode
		);

		// Write access informations too
		if (ret && aiToWrite)
		{
			if ((appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO) || getDESFireCommands()->authenticate(0))
			{							
				if (!dfAiToWrite->writeKey->isEmpty() && dfAiToUse->writeKey != dfAiToWrite->writeKey && dfAiToWrite->writeKeyno != 0x00)
				{
					try
					{
						if (appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO)
						{
							getDESFireCommands()->authenticate(dfAiToWrite->writeKeyno);
						}

						if (!getDESFireCommands()->changeKey(dfAiToWrite->writeKeyno, dfAiToWrite->writeKey))
						{
							THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
						}
					}
					catch(std::exception& ex)
					{
						THROW_EXCEPTION_WITH_LOG(CardException, std::string("Write key: ") + ex.what());
					}
				}

				if (!dfAiToWrite->readKey->isEmpty() && dfAiToUse->readKey != dfAiToWrite->readKey && dfAiToWrite->readKeyno != 0x00)
				{
					try
					{
						if (appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO)
						{
							getDESFireCommands()->authenticate(dfAiToWrite->readKeyno);
						}

						if (!getDESFireCommands()->changeKey(dfAiToWrite->readKeyno, dfAiToWrite->readKey))
						{
							THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
						}
					}
					catch(std::exception& ex)
					{
						THROW_EXCEPTION_WITH_LOG(CardException, std::string("Read key: ") + ex.what());
					}
				}

				if (!dfAiToWrite->masterApplicationKey->isEmpty() && dfAiToUse->masterApplicationKey != dfAiToWrite->masterApplicationKey)
				{
					try
					{
						if (appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO)
						{
							getDESFireCommands()->authenticate(0);
						}

						if (!getDESFireCommands()->changeKey(0, dfAiToWrite->masterApplicationKey))
						{
							THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
						}
					}
					catch(std::exception& ex)
					{
						THROW_EXCEPTION_WITH_LOG(CardException, std::string("Master application key: ") + ex.what());
					}
				}

				if (!dfAiToWrite->masterCardKey->isEmpty() && dfAiToUse->masterCardKey != dfAiToWrite->masterCardKey)
				{
					try
					{
						if (getDESFireCommands()->selectApplication(0x00))
						{
							if (getDESFireCommands()->authenticate(0))
							{
								if (!getDESFireCommands()->changeKey(0, dfAiToWrite->masterCardKey))
								{
									THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
								}
							}
						}
					}
					catch(std::exception& ex)
					{
						THROW_EXCEPTION_WITH_LOG(CardException, std::string("Master card key: ") + ex.what());
					}
				}
			}
		}

		return ret;
	}

	bool DESFireCardProvider::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		bool ret = false;

		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");
		EXCEPTION_ASSERT_WITH_LOG(data, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		boost::shared_ptr<DESFireAccessInfo> dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(aiToUse);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");
		if (aiToUse)
		{
			EXCEPTION_ASSERT_WITH_LOG(dfAiToUse, std::invalid_argument, "aiToUse must be a DESFireAccessInfo.");
		}
		else
		{
			dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(getChip()->getProfile()->createAccessInfo());
		}

		getChip()->getProfile()->clearKeys();		

		selectApplication(dfLocation);

		
		bool needLoadKey = true;
		EncryptionMode encMode = dfLocation->securityLevel;
		if (encMode == CM_UNKNOWN)
		{
			encMode = getEncryptionMode(aiToUse, static_cast<unsigned char>(dfLocation->file), true, &needLoadKey);
		}

		if (needLoadKey)
		{
			if (!dfAiToUse->readKey->isEmpty())
			{
				getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, dfAiToUse->readKeyno, dfAiToUse->readKey);
			}

			if (!getDESFireCommands()->authenticate(dfAiToUse->readKeyno))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_AUTHENTICATE);
			}
		}


		size_t reallen = getDESFireCommands()->readData(dfLocation->file,
			dfLocation->byte,
			dataLength,
			data,
			encMode);

		if (dataLength <= static_cast<size_t>(reallen))
		{
			ret = true;
		}

		return ret;
	}

	EncryptionMode DESFireCardProvider::getEncryptionMode(boost::shared_ptr<AccessInfo> aiToUse, unsigned char fileno, bool isReadMode, bool* needLoadKey = NULL)
	{
		EncryptionMode encMode = CM_ENCRYPT;
		DESFireCommands::FileSetting fileSetting;
		memset(&fileSetting, 0x00, sizeof(fileSetting));

		if (getDESFireCommands()->getFileSettings(fileno, fileSetting))
		{
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
		}

		return encMode;
	}	
	
	size_t DESFireCardProvider::getFileLength(unsigned char fileno)
	{
		size_t fileLength = 0x00;

		DESFireCommands::FileSetting fileSetting;
		memset(&fileSetting, 0x00, sizeof(fileSetting));
		if (getDESFireCommands()->getFileSettings(fileno, fileSetting))
		{
			switch (fileSetting.fileType)
			{
				case 0:
					{
						memcpy(&fileLength, fileSetting.type.dataFile.fileSize, sizeof(fileSetting.type.dataFile.fileSize));
					}
					break;
			}
		}

		return fileLength;
	}

	size_t DESFireCardProvider::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}

	boost::shared_ptr<DESFireChip> DESFireCardProvider::getDESFireChip()
	{
		return boost::dynamic_pointer_cast<DESFireChip>(getChip());
	}

	boost::shared_ptr<CardService> DESFireCardProvider::getService(CardServiceType serviceType)
	{
		boost::shared_ptr<CardService> service;

		switch (serviceType)
		{
		case CST_ACCESS_CONTROL:
			{
				service.reset(new AccessControlCardService(shared_from_this()));
			}
			break;
		case CST_NFC_TAG:
		  break;
		}

		if (!service)
		{
			service = CardProvider::getService(serviceType);
		}

		return service;
	}
}
