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

	bool DESFireStorageCardService::erase()
	{
		return getDESFireChip()->getDESFireCommands()->erase();
	}

	bool DESFireStorageCardService::erase(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse)
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
			ret = getDESFireChip()->getDESFireCommands()->erase();
		}
		// Format the application if MAK specified.
		else if (!dfAiToUse->masterApplicationKey->isEmpty())
		{
			getDESFireChip()->getDESFireProfile()->setKey(dfLocation->aid, 0, dfAiToUse->masterApplicationKey);

			if (!getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation->aid))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_SELECTAPPLICATION);
			}

			std::vector<int> files = getDESFireChip()->getDESFireCommands()->getFileIDs();
			for (std::vector<int>::const_iterator file = files.begin(); (file != files.end()); file++)
			{
				getDESFireChip()->getDESFireCommands()->deleteFile(*file);
			}

			if (!getDESFireChip()->getDESFireCommands()->changeKey(0, boost::shared_ptr<DESFireKey>(new DESFireKey(string("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00")))))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, string(EXCEPTION_MSG_CHANGEKEY));
			}

			ret = true;
		}
		// Otherwise format the file.
		else
		{
			if (!getDESFireChip()->getDESFireCommands()->selectApplication(dfLocation->aid))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_SELECTAPPLICATION);
			}

			size_t fileLength = getDESFireChip()->getDESFireCommands()->getFileLength(static_cast<unsigned char>(dfLocation->file));
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

	bool DESFireStorageCardService::writeData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, boost::shared_ptr<AccessInfo> aiToWrite, const void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
	{
		bool ret = false;

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
			std::vector<int> aids = getDESFireChip()->getDESFireCommands()->getApplicationIDs();

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
		unsigned int appMaxNbKeys = 3;

		bool needLoadKey = true;
		createArbo = true;
		try
		{
			getDESFireChip()->getDESFireCommands()->authenticate(0);

			getDESFireChip()->getDESFireCommands()->getKeySettings(appKeySettings, appMaxNbKeys);
			std::vector<int> files = getDESFireChip()->getDESFireCommands()->getFileIDs();

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
			if (!getDESFireChip()->getDESFireCommands()->authenticate(dfAiToUse->writeKeyno))
			{	
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_AUTHENTICATE);
			}
		}

		ret = getDESFireChip()->getDESFireCommands()->writeData(dfLocation->file,
			dfLocation->byte,
			dataLength,
			data,
			encMode
		);

		// Write access informations too
		if (ret && aiToWrite)
		{
			INFO_SIMPLE_("Starting to change keys...");
			if ((appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO) || getDESFireChip()->getDESFireCommands()->authenticate(0))
			{							
				if (!dfAiToWrite->writeKey->isEmpty() && dfAiToUse->writeKey != dfAiToWrite->writeKey && dfAiToWrite->writeKeyno != 0x00)
				{
					try
					{
						if (appKeySettings & KS_CHANGE_KEY_WITH_TARGETED_KEYNO)
						{
							getDESFireChip()->getDESFireCommands()->authenticate(dfAiToWrite->writeKeyno);
						}

						INFO_("Changing writeKey. div? %d", dfAiToWrite->writeKey->getDiversify());
						if (!getDESFireChip()->getDESFireCommands()->changeKey(dfAiToWrite->writeKeyno, dfAiToWrite->writeKey))
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
							getDESFireChip()->getDESFireCommands()->authenticate(dfAiToWrite->readKeyno);
						}

						INFO_("Changing readKey. div? %d", dfAiToWrite->readKey->getDiversify());
						if (!getDESFireChip()->getDESFireCommands()->changeKey(dfAiToWrite->readKeyno, dfAiToWrite->readKey))
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
							getDESFireChip()->getDESFireCommands()->authenticate(0);
						}

						INFO_("Changing masterApplicationKey. div? %d", dfAiToWrite->masterApplicationKey->getDiversify());
						if (!getDESFireChip()->getDESFireCommands()->changeKey(0, dfAiToWrite->masterApplicationKey))
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
						if (getDESFireChip()->getDESFireCommands()->selectApplication(0x00))
						{
							if (getDESFireChip()->getDESFireCommands()->authenticate(0))
							{
								INFO_("Changing masterCardKey. div? %d", dfAiToWrite->masterCardKey->getDiversify());
								if (!getDESFireChip()->getDESFireCommands()->changeKey(0, dfAiToWrite->masterCardKey))
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
		else
		{
			INFO_("Change keys skipped. ret: %d", ret);
		}

		return ret;
	}

	bool DESFireStorageCardService::readData(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> aiToUse, void* data, size_t dataLength, CardBehavior /*behaviorFlags*/)
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

			if (!getDESFireChip()->getDESFireCommands()->authenticate(dfAiToUse->readKeyno))
			{
				THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_AUTHENTICATE);
			}
		}


		size_t reallen = getDESFireChip()->getDESFireCommands()->readData(dfLocation->file,
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

	size_t DESFireStorageCardService::readDataHeader(boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, void* /*data*/, size_t /*dataLength*/)
	{
		return 0;
	}
}
