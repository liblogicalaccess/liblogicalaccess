/**
 * \file desfirestoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire storage card service.
 */

#include "desfirestoragecardservice.hpp"
#include "desfireev1location.hpp"

namespace logicalaccess
{
	DESFireStorageCardService::DESFireStorageCardService(boost::shared_ptr<Chip> chip)
		: StorageCardService(chip)
	{

	}

	void DESFireStorageCardService::erase()
	{
		boost::shared_ptr<DESFireCommands> cmd = getDESFireChip()->getDESFireCommands();
		cmd->selectApplication(0);
		cmd->authenticate(0);
		cmd->erase();
		
		cmd->changeKey(0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))));
	}

	void DESFireStorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
	{
		EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		boost::shared_ptr<DESFireLocation> dfLocation = boost::dynamic_pointer_cast<DESFireLocation>(location);
		boost::shared_ptr<DESFireAccessInfo> dfAiToUse = boost::dynamic_pointer_cast<DESFireAccessInfo>(aiToUse);

		EXCEPTION_ASSERT_WITH_LOG(dfLocation, std::invalid_argument, "location must be a DESFireLocation.");

		// Format the card if MCK specified.
		if (!dfAiToUse->masterCardKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(0, 0, dfAiToUse->masterCardKey);
			getDESFireChip()->getDESFireCommands()->erase();
		}
		// Format the application if MAK specified.
		else if (!dfAiToUse->masterApplicationKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, 0, dfAiToUse->masterApplicationKey);

			getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation->aid);

			std::vector<unsigned char> files = getDESFireChip()->getDESFireCommands()->getFileIDs();
			for (std::vector<unsigned char>::const_iterator file = files.cbegin(); (file != files.cend()); file++)
			{
				getDESFireChip()->getDESFireCommands()->deleteFile(*file);
			}

			getDESFireChip()->getDESFireCommands()->changeKey(0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"))));
		}
		// Otherwise format the file.
		else
		{
			getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation->aid);

			size_t fileLength = getDESFireChip()->getDESFireCommands()->getFileLength(static_cast<unsigned char>(dfLocation->file));
			unsigned char* buf =  new unsigned char[fileLength];
			memset(buf, 0x00, fileLength);

			boost::shared_ptr<AccessInfo> ai;			
			try
			{
				writeData(location, aiToUse, ai, buf, fileLength, CB_DEFAULT);
			}
			catch(std::exception&)
			{
				delete[] buf;
				throw;
			}

			delete[] buf;			
		}
	}

	void DESFireStorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		INFO_SIMPLE_("Starting write data...");

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

		getDESFireChip()->getDESFireCommands()->selectApplication(0);

		bool createArbo = true;
		try
		{
			getDESFireChip()->getDESFireCommands()->authenticate(0);

			// Create application if doesn't exist
			std::vector<unsigned int> aids = getDESFireChip()->getDESFireCommands()->getApplicationIDs();

			if (aiToWrite)
			{
				for (std::vector<unsigned int>::const_iterator aid = aids.cbegin(); (aid != aids.cend()) && createArbo; aid++)
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

				getDESFireChip()->getDESFireCommands()->createApplication(dfLocation, KS_DEFAULT, maxNbKeys);
			}
		}
		catch(std::exception&)
		{
			createArbo = false;
		}

		getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation);
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
		unsigned char appMaxNbKeys = 3;

		bool needLoadKey = true;
		createArbo = true;
		try
		{
			getDESFireChip()->getDESFireCommands()->authenticate(0);

			getDESFireChip()->getDESFireCommands()->getKeySettings(appKeySettings, appMaxNbKeys);
			std::vector<unsigned char> files = getDESFireChip()->getDESFireCommands()->getFileIDs();

			if (aiToWrite)
			{
				for (std::vector<unsigned char>::const_iterator file = files.cbegin(); (file != files.cend()) && createArbo; file++)
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

				getDESFireChip()->getDESFireCommands()->createStdDataFile(dfLocation, rights, static_cast<int>(dataLength) + dfLocation->byte);
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
			encMode = getDESFireChip()->getDESFireCommands()->getEncryptionMode(aiToUse, static_cast<unsigned char>(dfLocation->file), false, &needLoadKey);
		}

		if (needLoadKey)
		{
			getDESFireChip()->getDESFireCommands()->authenticate(dfAiToUse->writeKeyno);
		}

		getDESFireChip()->getDESFireCommands()->writeData(dfLocation->file,
			dfLocation->byte,
			static_cast<unsigned int>(dataLength),
			data,
			encMode
		);

		// Write access informations too
		if (aiToWrite)
		{
			INFO_SIMPLE_("Starting to change keys...");
			if ((appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO))
			{		
				getDESFireChip()->getDESFireCommands()->authenticate(0);
				if (!dfAiToWrite->writeKey->isEmpty() && dfAiToUse->writeKey != dfAiToWrite->writeKey && dfAiToWrite->writeKeyno != 0x00)
				{
					try
					{
						if (appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO)
						{
							getDESFireChip()->getDESFireCommands()->authenticate(dfAiToWrite->writeKeyno);
						}

						getDESFireChip()->getDESFireCommands()->changeKey(dfAiToWrite->writeKeyno, dfAiToWrite->writeKey);
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
							getDESFireChip()->getDESFireCommands()->authenticate(dfAiToWrite->readKeyno);
						}

						INFO_SIMPLE_("Changing readKey.");
						getDESFireChip()->getDESFireCommands()->changeKey(dfAiToWrite->readKeyno, dfAiToWrite->readKey);
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
							getDESFireChip()->getDESFireCommands()->authenticate(0);
						}

						INFO_SIMPLE_("Changing masterApplicationKey.");
						getDESFireChip()->getDESFireCommands()->changeKey(0, dfAiToWrite->masterApplicationKey);
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
						getDESFireChip()->getDESFireCommands()->selectApplication(0x00);
						getDESFireChip()->getDESFireCommands()->authenticate(0);
						
						INFO_("Changing masterCardKey. div? %d", dfAiToWrite->masterCardKey->getKeyDiversification() == NULL);
						getDESFireChip()->getDESFireCommands()->changeKey(0, dfAiToWrite->masterCardKey);
					}
					catch(std::exception& ex)
					{
						THROW_EXCEPTION_WITH_LOG(CardException, std::string("Master card key: ") + ex.what());
					}
				}
			}
		}
	}

	void DESFireStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
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

		getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation);

		
		bool needLoadKey = true;
		EncryptionMode encMode = dfLocation->securityLevel;
		if (encMode == CM_UNKNOWN)
		{
			encMode = getDESFireChip()->getDESFireCommands()->getEncryptionMode(aiToUse, static_cast<unsigned char>(dfLocation->file), true, &needLoadKey);
		}

		if (needLoadKey)
		{
			if (!dfAiToUse->readKey->isEmpty())
			{
				getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, dfAiToUse->readKeyno, dfAiToUse->readKey);
			}

			getDESFireChip()->getDESFireCommands()->authenticate(dfAiToUse->readKeyno);
		}


		size_t reallen = getDESFireChip()->getDESFireCommands()->readData(dfLocation->file,
			dfLocation->byte,
			static_cast<unsigned int>(dataLength),
			data,
			encMode);
	}

	unsigned int DESFireStorageCardService::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}
}
