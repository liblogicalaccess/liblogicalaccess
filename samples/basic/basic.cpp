/**
 * \file main.cpp
 * \author Maxime Chamley <maxime.chamley@islog.eu>
 * \brief Main file for the basic sample of the liblogicalaccess.
 */

#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include "logicalaccess/DynLibrary/LibraryManager.h"
#include "logicalaccess/ReaderProviders/ReaderConfiguration.h"
//#include "ReaderProviders/PCSC/PCSCReaderProvider.h"
//#include <SmartIDReaderProvider.h>
/*#include <SCIELReaderProvider.h>
#include <DeisterReaderProvider.h>
#include <RFIDeasReaderProvider.h>
#include <AdmittoReaderProvider.h>
#include <AxessTMCLegicReaderProvider.h>
#include <AxessTMC13ReaderProvider.h>
#include <PromagReaderProvider.h>
#include <A3MLGM5600ReaderProvider.h>
#include <STidSTRReaderProvider.h>
#include <ElatecReaderProvider.h>
#include <IdOnDemandReaderProvider.h>*/
#include "logicalaccess/Cards/CardProvider.h"
/*#include <DESFireEV1Commands.h>
#include <Mifare1KChip.h>
#include <Mifare4KChip.h>
#include <DESFireChip.h>
#include <DESFireEV1Location.h>
#include <DESFireAccessInfo.h>
#include <HIDiClass32KS_8x2_16Chip.h>
#include <HIDiClass8x2KSChip.h>
#include <Wiegand26Format.h>
#include <Wiegand37Format.h>
#include <Wiegand37WithFacilityFormat.h>
#include <Wiegand34Format.h>
#include <Wiegand34WithFacilityFormat.h>
#include <RawFormat.h>
#include <Corporate1000Format.h>
#include <TagItChip.h>
#include <DataClockFormat.h>
#include <TwicChip.h>
#include <FASCN200BitFormat.h>
#include <Wiegand37WithFacilityRightParity2Format.h>
#include <OmnikeyHIDiClassReaderCardAdapter.h>
#include <CustomFormat.h>
#include <ReaderFormatComposite.h>
#include <MifareUltralightChip.h>
#include <AccessControlCardService.h>*/

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

using std::cout;
using std::cerr;
using std::cin;
using std::hex;
using std::setfill;
using std::setw;
using std::endl;
using std::flush;
using std::string;
using std::ostringstream;
using std::istringstream;

using namespace LOGICALACCESS;

// Trash version
void JustDoIt(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	//// READ/WRITE DATA TO A CARD
	boost::shared_ptr<Location> location;
	boost::shared_ptr<AccessInfo> aiToUse;
	boost::shared_ptr<AccessInfo> aiToWrite;

	///* -------------------------------- */
	///* Example with Mifare Classic chip */
	///* -------------------------------- */

	//// We want to write data on sector 1.
	//boost::shared_ptr<MifareLocation> mlocation(new MifareLocation());
	//mlocation->sector = 1;
	//mlocation->block = 0;
	//location = mlocation;

	//// Key to use for sector authentication
	//boost::shared_ptr<MifareAccessInfo> maiToUse(new MifareAccessInfo());
	//maiToUse->keyA->fromString("ff ff ff ff ff ff");
	//aiToUse = maiToUse;

	//// Change the sector key with the following key
	//boost::shared_ptr<MifareAccessInfo> maiToWrite(new MifareAccessInfo());
	//maiToWrite->keyA->fromString("aa aa aa aa aa aa");
	//maiToWrite->keyB->fromString("bb bb bb bb bb bb");
	//maiToWrite->sab.setAReadBWriteConfiguration();
	//aiToWrite = maiToWrite;

	///* -------------------------------- */
	///* -------------------------------- */
	///* -------------------------------- */

	/* -------------------------------- */
	/* Example with Mifare DESFire EV1 chip */
	/* -------------------------------- */

	// We want to write data on sector 1.
	/*boost::shared_ptr<DESFireLocation> dlocation(new DESFireLocation());
	dlocation->aid = 0x513;
	dlocation->file = 0;
	dlocation->securityLevel = CM_ENCRYPT;		*/			
	//dlocation->useEV1 = true;
	//dlocation->cryptoMethod = DF_KEY_DES;
	//location = dlocation;

	// Key to use for authentication
	//aiToUse = chip->getProfile()->createAccessInfo();

	// Change the key with the following key
	/*boost::shared_ptr<DESFireAccessInfo> daiToWrite(new DESFireAccessInfo());
	daiToWrite->masterApplicationKey->setKeyType(DF_KEY_DES //dlocation->cryptoMethod);
	daiToWrite->readKey->setKeyType(DF_KEY_DES);
	daiToWrite->readKey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	daiToWrite->readKeyno = 1;
	daiToWrite->writeKey->setKeyType(DF_KEY_DES);
	daiToWrite->writeKey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	daiToWrite->writeKeyno = 2;
	daiToWrite->masterApplicationKey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03");
	daiToWrite->masterApplicationKey->setKeyType(DF_KEY_DES);
	aiToWrite = daiToWrite;*/

	/*boost::shared_ptr<DESFireEV1Commands> commands = boost::dynamic_pointer_cast<DESFireEV1Commands>(chip->getCardProvider()->getCommands());

	DESFireKeySettings settings;
	unsigned int maxNbKeys = 0;
	DESFireKeyType keyType;
	commands->getKeySettings(settings, maxNbKeys, keyType);
	commands->createApplication(dlocation->aid, KS_DEFAULT, 3, FIDS_NO_ISO_FID, DF_KEY_DES);*/

	/* -------------------------------- */
	/* -------------------------------- */
	/* -------------------------------- */

	/* -------------------------------- */
	/* Example with HID iClass chip */
	/* -------------------------------- */

	// We want to read data on page 1
	/*boost::shared_ptr<HIDiClassLocation> ilocation(new HIDiClassLocation());
	ilocation->book = 0;
	ilocation->page = 1;
	ilocation->block = 6;
	ilocation->zoneArea = KT_KEY_KD;
	location = ilocation;

	boost::shared_ptr<HIDiClassAccessInfo> iAi(new HIDiClassAccessInfo());
	iAi->zoneAreaKey->fromString("C0 C1 C2 C3 C4 C5 C6 C7");
	aiToUse = iAi;*/

	// Data to write
	/*unsigned char writedata[10];
	memset(writedata, 0x00, sizeof(writedata));
	writedata[0] = 0x11;
	writedata[9] = 0xff;*/



	// Data read
	//unsigned char readdata[4];
	//memset(readdata, 0x00, sizeof(readdata));


	// Write data on the specified location with the specified key
	//cp->writeData(location, aiToUse, aiToWrite, writedata, sizeof(writedata), CB_DEFAULT);

	//// We read the data on the same location. Remember, the key is now changed.
	//cp->readData(location, aiToWrite, readdata, sizeof(readdata), CB_DEFAULT);

	// ENCODE A CARD WITH A FORMAT
	//boost::shared_ptr<Format> format;
					

	/* -------------------------------- */
	/* Example with Wiegand 26 format   */
	/* -------------------------------- */

	/*boost::shared_ptr<Wiegand26Format> w26format(new Wiegand26Format());
	w26format->setFacilityCode(50);
	w26format->setUid(1000);
	format = w26format;*/

	/*boost::shared_ptr<CustomFormat> cformat(new CustomFormat());
	boost::dynamic_pointer_cast<XmlSerializable>(cformat)->unSerialize("<CustomFormat type=\"16\"><Name>Custom</Name><Fields><BinaryDataField><Name>MatriculeCtrl</Name><Position>0</Position><IsFixedField>false</IsFixedField><IsIdentifier>false</IsIdentifier><DataRepresentation>4</DataRepresentation><DataType>3</DataType><Length>8</Length><Value></Value><Padding></Padding></BinaryDataField><NumberDataField><Name>NoCare</Name><Position>8</Position><IsFixedField>false</IsFixedField><IsIdentifier>false</IsIdentifier><DataRepresentation>4</DataRepresentation><DataType>3</DataType><Length>96</Length><Value>0</Value></NumberDataField><BinaryDataField><Name>MatriculeID</Name><Position>104</Position><IsFixedField>false</IsFixedField><IsIdentifier>false</IsIdentifier><DataRepresentation>4</DataRepresentation><DataType>3</DataType><Length>24</Length><Value></Value><Padding></Padding></BinaryDataField></Fields></CustomFormat>", "");
	format = cformat;*/

	/* -------------------------------- */
	/* -------------------------------- */
	/* -------------------------------- */

	/*boost::shared_ptr<AccessControlCardService> acService = boost::dynamic_pointer_cast<AccessControlCardService>(cp->getService(CST_ACCESS_CONTROL));
	if (acService)
	{
		// Write the format to the card. We use object from the previous snippet code.
		acService->writeFormat(format, location, aiToWrite, boost::shared_ptr<AccessInfo>());

		// Read default identifier from the card
		format = acService->readFormat(format, location, aiToUse);
		if (format)
		{
			cout << "Format read. Identifier: " << format->getIdentifier().toHex() << endl;
		}
	}*/

	//// LOW LEVEL CHIP COMMAND
	///* -------------------------------- */
	///* Example with Mifare Classic chip */
	///* -------------------------------- */

	//boost::shared_ptr<MifareCardProvider> mifarecp = boost::dynamic_pointer_cast<MifareCardProvider>(cp);

	//boost::shared_ptr<MifareKey> madKeyA(new MifareKey("a0 a1 a2 a3 a4 a5"));
	//// Get the referenced sector from the MAD AID
	//unsigned int sector = mifarecp->getSectorFromMAD(0x4182, madKeyA);

	///* -------------------------------- */
	///* -------------------------------- */
	///* -------------------------------- */

	//// HANDLE EXCEPTION
	//try
	//{
	//	// DO CONTACT/CONTACTLESS STUFF HERE
	//	// DO CONTACT/CONTACTLESS STUFF HERE
	//	// DO CONTACT/CONTACTLESS STUFF HERE
	//}
	//catch(LibLOGICALACCESSException& ex)
	//{
	//	std::cout << "LibLogicalAccess exception: " << ex.getMessage() << std::endl;
	//}

	//// ITERATE THE CHIP MEMORY AREAS
	//// Get the root node
	//boost::shared_ptr<LocationNode> rootNode = chip->getRootLocationNode();
	//// Get childrens of this node
	//std::vector<boost::shared_ptr<LocationNode> > childs = rootNode->getChildrens();
	//for (std::vector<boost::shared_ptr<LocationNode> >::iterator i = childs.begin(); i != childs.end(); ++i)
	//{
	//	// Display node information
	//	std::cout << "Size of node " << (*i)->getName() << ": " << ((*i)->getLength() * (*i)->getUnit()) << " bytes";

	//	// Get the associated location for that node (to read or write data for example)
	//	boost::shared_ptr<Location> childlocation = (*i)->getLocation();
	//}

	//// Use The Composite
	//boost::shared_ptr<ReaderFormatComposite> rfc(new ReaderFormatComposite());
	//rfc->setReaderConfiguration(readerConfig);
	//boost::shared_ptr<CardsFormatComposite> composite(new CardsFormatComposite());
	//// DESFire
	//boost::shared_ptr<Wiegand26Format> format(new Wiegand26Format());
	//boost::shared_ptr<DESFireLocation> location(new DESFireLocation());
	//boost::shared_ptr<DESFireAccessInfo> aiToUse(new DESFireAccessInfo());
	//aiToUse->readKey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02");
	//aiToUse->readKeyno = 2;
	//aiToUse->writeKey->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01");
	//aiToUse->writeKeyno = 1;
	//composite->addFormatForCard(CT_DESFIRE, format, location, aiToUse);
	//rfc->setCardsFormatComposite(composite);

	//std::string xmlrfc = boost::dynamic_pointer_cast<XmlSerializable>(rfc)->serialize();
	//cout << xmlrfc << std::endl;
}


// MAD Test
void JustDoIt_MAD(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<MifareCardProvider> mcp = boost::dynamic_pointer_cast<MifareCardProvider>(cp);
					
	if (mcp)
	{
		unsigned char data[48];
		memset(data, 0x00, sizeof(data));

		boost::shared_ptr<MifareLocation> location(new MifareLocation());
		boost::shared_ptr<MifareAccessInfo> aiToWrite(new MifareAccessInfo());


		location->sector = 1;
		location->block = 0;
		location->useMAD = true;
		location->aid = 0x482A;
		aiToWrite->keyA->fromString("ff ff ff ff ff fa");
		aiToWrite->keyB->fromString("ff ff ff ff ff fb");
		aiToWrite->sab.setAReadBWriteConfiguration();
		aiToWrite->useMAD = true;
		aiToWrite->madKeyA->fromString("a0 a1 a2 a3 a4 a5");
		aiToWrite->madKeyB->fromString("ff ff ff ff ff fb");

		data[0] = 0x01;
		data[47] = 0x47;

		if (mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Data + MAD writted." << std::endl;
		}
		else
		{
			std::cout << "Cannot write data + MAD !" << std::endl;
		}
	}*/
}

// Vigik test

void JustDoIt_VIGIK(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<MifareCardProvider> mcp = boost::dynamic_pointer_cast<MifareCardProvider>(cp);
					
	if (mcp)
	{
		unsigned char data[48];
		memset(data, 0x00, sizeof(data));

		boost::shared_ptr<MifareLocation> location(new MifareLocation());
		boost::shared_ptr<MifareAccessInfo> aiToWrite(new MifareAccessInfo());


		location->sector = 0;
		location->block = 1;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("a2 2a e1 29 c0 13");
		data[0] = 0xB1;
		data[1] = 0x52;
		data[2] = 0x11;
		data[3] = 0x2B;
		data[4] = 0xFE;
		data[5] = 0xEF;
		data[6] = 0xF8;
		data[7] = 0xBF;
		data[8] = 0xF5;
		data[9] = 0xFE;
		data[10] = 0x62;
		data[11] = 0xC9;
		data[12] = 0x55;
		data[13] = 0xFC;
		data[14] = 0x56;
		data[15] = 0xF2;
		data[16] = 0x5F;
		data[17] = 0x5A;
		data[18] = 0x8D;
		data[19] = 0x0B;
		data[20] = 0x9E;
		data[21] = 0xAF;
		data[22] = 0xD7;
		data[23] = 0xF1;
		data[24] = 0x10;
		data[25] = 0x00;
		data[26] = 0x00;
		data[27] = 0x00;
		data[28] = 0x00;
		data[29] = 0x00;
		data[30] = 0x06;
		data[31] = 0x00;
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, 32, CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}


		location->sector = 1;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("49 fa e4 e3 84 9f");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 2;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("38 fc f3 30 72 e0");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 3;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("8a d5 51 7b 4b 18");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 4;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("50 93 59 f1 31 b1");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 5;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("6c 78 92 8e 13 17");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 6;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("aa 07 20 01 83 38");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 7;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("a6 ca c2 88 64 12");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 8;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("62 d0 c4 24 ed 8e");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 9;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("e6 4a 98 6a 5d 94");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 10;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("8f a1 d6 01 d0 a2");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 11;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("89 34 73 50 bd 36");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 12;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("66 d2 b7 dc 39 ef");
		memset(data, 0xff, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 13;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("6b c1 e1 ae 54 7d");
		data[0] = 0xF1;
		data[1] = 0x82;
		data[2] = 0xD1;
		data[3] = 0x50;
		data[4] = 0xD4;
		data[5] = 0xB1;
		data[6] = 0xD1;
		data[7] = 0xA0;
		data[8] = 0x4D;
		data[9] = 0x2B;
		data[10] = 0x3C;
		data[11] = 0x08;
		data[12] = 0x53;
		data[13] = 0x63;
		data[14] = 0x11;
		data[15] = 0xAA;
		data[16] = 0x55;
		data[17] = 0xA6;
		data[18] = 0x1F;
		data[19] = 0xDA;
		data[20] = 0x44;
		data[21] = 0x7C;
		data[22] = 0x70;
		data[23] = 0xC8;
		data[24] = 0xB8;
		data[25] = 0x2E;
		data[26] = 0x34;
		data[27] = 0x18;
		data[28] = 0x31;
		data[29] = 0x62;
		data[30] = 0xC5;
		data[31] = 0xC6;
		data[32] = 0x37;
		data[33] = 0x8A;
		data[34] = 0x46;
		data[35] = 0xBA;
		data[36] = 0x74;
		data[37] = 0xE8;
		data[38] = 0x44;
		data[39] = 0xC5;
		data[40] = 0x45;
		data[41] = 0x23;
		data[42] = 0x2C;
		data[43] = 0x28;
		data[44] = 0x51;
		data[45] = 0xA2;
		data[46] = 0x62;
		data[47] = 0x9B;
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 14;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("22 72 9a 9b d4 0f");
		memset(data, 0x00, sizeof(data));
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		location->sector = 15;
		location->block = 0;
		aiToWrite->keyA->fromString("48 45 58 41 43 54");
		aiToWrite->keyB->fromString("48 45 58 41 43 54");
		data[0] = 0x48;
		data[1] = 0x45;
		data[2] = 0x58;
		data[3] = 0x41;
		data[4] = 0x43;
		data[5] = 0x54;
		data[6] = 0x20;
		data[7] = 0x2D;
		data[8] = 0x20;
		data[9] = 0x43;
		data[10] = 0x4F;
		data[11] = 0x47;
		data[12] = 0x45;
		data[13] = 0x4C;
		data[14] = 0x45;
		data[15] = 0x43;
		data[16] = 0x64;
		data[17] = 0x65;
		data[18] = 0x70;
		data[19] = 0x6F;
		data[20] = 0x74;
		data[21] = 0x20;
		data[22] = 0x6C;
		data[23] = 0x65;
		data[24] = 0x67;
		data[25] = 0x61;
		data[26] = 0x6C;
		data[27] = 0x20;
		data[28] = 0x49;
		data[29] = 0x4E;
		data[30] = 0x50;
		data[31] = 0x49;
		data[32]= 0x6E;
		data[33] = 0x65;
		data[34] = 0x20;
		data[35] = 0x70;
		data[36] = 0x61;
		data[37] = 0x73;
		data[38] = 0x20;
		data[39] = 0x75;
		data[40] = 0x74;
		data[41] = 0x69;
		data[42] = 0x6C;
		data[43] = 0x69;
		data[44] = 0x73;
		data[45] = 0x65;
		data[46] = 0x72;
		data[47] = 0x2E;
		if (!mcp->writeData(location, boost::shared_ptr<AccessInfo>(), aiToWrite, data, sizeof(data), CB_DEFAULT))
		{
			std::cout << "Cannot create sector " << location->sector << std::endl;
		}

		std::cout << "Vigik card creation ended." << std::endl;
	}*/
}

// Gendarmerie version
void JustDoIt_Gendarmerie(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<DESFireCommands> dfcmd = boost::dynamic_pointer_cast<DESFireCommands>(cp->getCommands());
	boost::shared_ptr<DESFireEV1Commands> dfev1cmd = boost::dynamic_pointer_cast<DESFireEV1Commands>(cp->getCommands());
	boost::shared_ptr<DESFireProfile> dfprofile = boost::dynamic_pointer_cast<DESFireProfile>(profile);

	boost::shared_ptr<DESFireKey> dfPICCKey(new DESFireKey("AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA"));
	dfPICCKey->setKeyVersion(0);
	dfPICCKey->setKeyType(DF_KEY_AES);

	dfcmd->selectApplication(0);

	try
	{
		dfcmd->authenticate(0);
		dfcmd->erase(false);
		try
		{
			dfev1cmd->setConfiguration(true, true);
		}
		catch(std::exception&)
		{
		}
		dfcmd->changeKey(0, dfPICCKey);
		dfprofile->setKey(0, 0, dfPICCKey);
		dfcmd->authenticate(0);
	}
	catch(std::exception&)
	{
		dfprofile->setKey(0, 0, dfPICCKey);
		dfcmd->authenticate(0);
		dfcmd->erase(false);
	}
	dfcmd->changeKeySettings(static_cast<DESFireKeySettings>(KS_ALLOW_CHANGE_MK | KS_CONFIGURATION_CHANGEABLE));

	boost::shared_ptr<DESFireKey> dfTransportKey(new DESFireKey("BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB"));
	dfTransportKey->setKeyVersion(1);
	dfTransportKey->setKeyType(DF_KEY_AES);

	boost::shared_ptr<DESFireKey> dfReadKey(new DESFireKey("8b 95 b4 f7 9f 12 0b 1b 5a ec bf 6d e2 56 5d 60"));
	dfReadKey->setKeyVersion(1);
	dfReadKey->setKeyType(DF_KEY_AES);

	dfcmd->authenticate(0);

	int aid = 0x3F002F;

	unsigned char data[16];
	memset(data, 0x00, sizeof(data));
	//data[14] = 0x03;
	//data[15] = 0x6C;
	//data[14] = 0x0E;
	//data[15] = 0x07;
	//data[14] = 0x01;
	//data[15] = 0xC8;
	data[14] = 0x1A;
	data[15] = 0x57;

	dfev1cmd->createApplication(aid,
		static_cast<DESFireKeySettings>(KS_ALLOW_CHANGE_MK | KS_CONFIGURATION_CHANGEABLE | KS_CHANGE_KEY_WITH_TARGETED_KEYNO),
		14,
		FIDS_NO_ISO_FID,
		DF_KEY_AES);

	dfcmd->selectApplication(aid);

	
	DESFireAccessRights accessRights;
	accessRights.readAccess = AR_KEY1;
	accessRights.writeAccess = AR_KEY0;
	accessRights.readAndWriteAccess = AR_KEY0;
	accessRights.changeAccess = AR_KEY0;

	dfcmd->createStdDataFile(0, CM_ENCRYPT, accessRights, sizeof(data));
	dfcmd->authenticate(0);
	dfcmd->writeData(0, 0, sizeof(data), data, CM_ENCRYPT);
	*/
	/*for (int i = 0; i < 14; ++i)
	{
		dfcmd->authenticate(i);
		dfcmd->changeKey(i, dfTransportKey);
		dfprofile->setKey(aid, i, dfTransportKey);
	}*/

	/*
	dfcmd->authenticate(1);
	dfcmd->changeKey(1, dfReadKey);
	dfprofile->setKey(aid, 1, dfReadKey);
	*/
}

// Gendarmerie version
void JustDoIt_GendarmeriePLAIN(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<DESFireCommands> dfcmd = boost::dynamic_pointer_cast<DESFireCommands>(cp->getCommands());
	boost::shared_ptr<DESFireEV1Commands> dfev1cmd = boost::dynamic_pointer_cast<DESFireEV1Commands>(cp->getCommands());
	boost::shared_ptr<DESFireProfile> dfprofile = boost::dynamic_pointer_cast<DESFireProfile>(profile);

	boost::shared_ptr<DESFireKey> dfPICCKey(new DESFireKey("AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA"));
	dfPICCKey->setKeyVersion(0);
	dfPICCKey->setKeyType(DF_KEY_AES);

	dfcmd->selectApplication(0);

	try
	{
		dfcmd->authenticate(0);
		dfcmd->erase(false);
		try
		{
			dfev1cmd->setConfiguration(true, true);
		}
		catch(std::exception&)
		{
		}
		dfcmd->changeKey(0, dfPICCKey);
		dfprofile->setKey(0, 0, dfPICCKey);
		dfcmd->authenticate(0);
	}
	catch(std::exception&)
	{
		dfprofile->setKey(0, 0, dfPICCKey);
		dfcmd->authenticate(0);
		dfcmd->erase(false);
	}
	dfcmd->changeKeySettings(static_cast<DESFireKeySettings>(KS_ALLOW_CHANGE_MK | KS_CONFIGURATION_CHANGEABLE));

	boost::shared_ptr<DESFireKey> dfTransportKey(new DESFireKey("BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB"));
	dfTransportKey->setKeyVersion(1);
	dfTransportKey->setKeyType(DF_KEY_DES);

	int aid = 0x3F002F;

	unsigned char data[16];
	memset(data, 0x00, sizeof(data));
	//data[14] = 0x03;
	//data[15] = 0x6C;
	//data[14] = 0x0E;
	//data[15] = 0x07;
	data[14] = 0x01;
	data[15] = 0xC8;
	//data[14] = 0x1A;
	//data[15] = 0x57;

	dfcmd->createApplication(aid,
		static_cast<DESFireKeySettings>(KS_ALLOW_CHANGE_MK | KS_CONFIGURATION_CHANGEABLE | KS_CHANGE_KEY_WITH_TARGETED_KEYNO),
		14);

	dfcmd->selectApplication(aid);
	dfcmd->authenticate(0);
	dfcmd->changeKey(0, dfTransportKey);
	dfprofile->setKey(aid, 0, dfTransportKey);
	dfcmd->authenticate(0);

	DESFireAccessRights accessRights;
	accessRights.readAccess = AR_FREE;
	accessRights.writeAccess = AR_KEY0;
	accessRights.readAndWriteAccess = AR_KEY0;
	accessRights.changeAccess = AR_KEY0;

	dfcmd->createStdDataFile(0, CM_PLAIN, accessRights, sizeof(data));
	dfcmd->authenticate(0);
	dfcmd->writeData(0, 0, sizeof(data), data, CM_PLAIN);

	
	dfcmd->authenticate(1);
	dfcmd->changeKey(1, dfReadKey);
	dfprofile->setKey(aid, 1, dfReadKey);
	*/
}

// Mifare read test
void JustDoIt_MifareRead(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<MifareCardProvider> mcp = boost::dynamic_pointer_cast<MifareCardProvider>(cp);
	boost::shared_ptr<MifareProfile> mprofile = boost::dynamic_pointer_cast<MifareProfile>(profile);

	unsigned char data[48];
	memset(data, 0x00, sizeof(data));

	boost::shared_ptr<MifareLocation> location(new MifareLocation());
	location->sector = 1;

	boost::shared_ptr<MifareAccessInfo> aiToUse;
	aiToUse = boost::dynamic_pointer_cast<MifareAccessInfo>(mprofile->createAccessInfo());

	if (mcp->readData(location, aiToUse, data, sizeof(data), CB_DEFAULT))
	{
		std::cout << "Sector read successfully." << std::endl;
	}*/
}

void JustDoIt_EncodeGendarmerie(boost::shared_ptr<Chip>& chip, boost::shared_ptr<CardProvider>& cp, boost::shared_ptr<Profile>& profile)
{
	/*boost::shared_ptr<DESFireCommands> dfcmd = boost::dynamic_pointer_cast<DESFireCommands>(cp->getCommands());
	boost::shared_ptr<DESFireEV1Commands> dfev1cmd = boost::dynamic_pointer_cast<DESFireEV1Commands>(cp->getCommands());
	boost::shared_ptr<DESFireProfile> dfprofile = boost::dynamic_pointer_cast<DESFireProfile>(profile);

	boost::shared_ptr<AccessControlCardService> acService = boost::dynamic_pointer_cast<AccessControlCardService>(cp->getService(CST_ACCESS_CONTROL));

	boost::shared_ptr<Wiegand26Format> format(new Wiegand26Format());
	format->setFacilityCode(50);
	format->setUid(1234);

	boost::shared_ptr<DESFireEV1Location> location(new DESFireEV1Location());
	location->aid = 0x3F002F;
	location->file = 0;
	location->securityLevel = CM_ENCRYPT;
	location->cryptoMethod = DF_KEY_AES;

	boost::shared_ptr<DESFireKey> dfTransportKey(new DESFireKey("BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB"));
	dfTransportKey->setKeyVersion(1);
	dfTransportKey->setKeyType(DF_KEY_AES);

	boost::shared_ptr<DESFireKey> dfKey0(new DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03"));
	dfKey0->setKeyVersion(0);
	dfKey0->setKeyType(DF_KEY_AES);

	boost::shared_ptr<DESFireKey> dfKey1(new DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01"));
	dfKey1->setKeyVersion(0);
	dfKey1->setKeyType(DF_KEY_AES);

	boost::shared_ptr<DESFireKey> dfKey2(new DESFireKey("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02"));
	dfKey2->setKeyVersion(0);
	dfKey2->setKeyType(DF_KEY_AES);

	boost::shared_ptr<DESFireAccessInfo> aiToUse(new DESFireAccessInfo());
	aiToUse->masterApplicationKey->setData(dfTransportKey->getData());
	aiToUse->masterApplicationKey->setKeyType(dfTransportKey->getKeyType());
	aiToUse->readKey->setData(dfTransportKey->getData());
	aiToUse->readKey->setKeyType(dfTransportKey->getKeyType());
	aiToUse->readKeyno = 2;
	aiToUse->writeKey->setData(dfTransportKey->getData());
	aiToUse->writeKey->setKeyType(dfTransportKey->getKeyType());
	aiToUse->writeKeyno = 2;

	boost::shared_ptr<DESFireAccessInfo> aiToWrite(new DESFireAccessInfo());
	aiToWrite->masterApplicationKey->setData(dfKey0->getData());
	aiToWrite->masterApplicationKey->setKeyType(dfKey0->getKeyType());
	aiToWrite->readKey->setData(dfKey2->getData());
	aiToWrite->readKey->setKeyType(dfKey2->getKeyType());
	aiToWrite->readKeyno = 2;
	aiToWrite->writeKey->setData(dfKey1->getData());
	aiToWrite->writeKey->setKeyType(dfKey1->getKeyType());
	aiToWrite->writeKeyno = 1;

	acService->writeFormat(format, location, aiToUse, aiToWrite);*/
}

void JustDoIt_TestSerialize()
{
	/*boost::shared_ptr<DESFireAccessInfo> ai(new DESFireAccessInfo());
	ai->generateInfos();

	std::string filename("c:\\ai.xml");
	ai->serializeToFile(filename);
	cout << "AI serialized to file " << filename << "." << std::endl;

	ai.reset(new DESFireAccessInfo());
	ai->unSerializeFromFile(filename);
	cout << "AI unserialized from file " << filename << "." << std::endl;*/
}

   void Deleter( LOGICALACCESS::ReaderProvider* ptr)
   {
   }

/**
 * \brief The application entry point.
 * \param argc The arguments count.
 * \param argv The arguments.
 */
int main(int , char**)
{
	try
	{
		string readerSerialPort;

#ifdef LINUX
		readerSerialPort = "/dev/ttyUSB1";
#else
		readerSerialPort = "COM6";
#endif

		// Reader configuration object to store reader provider and reader unit selection.
		boost::shared_ptr<ReaderConfiguration> readerConfig(new ReaderConfiguration());

		// PC/SC
		/*getprovider pv;
		*(void**)(&pv) = LibraryManager::getInstance()->getFctFromName("getPCSCReader", LibraryManager::READERS_TYPE);
		readerConfig->setReaderProvider(LibraryManager::getInstance()->getReaderProvider("PCSC"));*/

		if (readerConfig->getReaderProvider()->getReaderList().size() == 0)
		{
			cerr << "No readers on this system." << endl;
			return EXIT_FAILURE;
		}
		else
		{
			cout << readerConfig->getReaderProvider()->getReaderList().size() << " readers on this system." << endl;			
		}
		// Create the default reader unit. On PC/SC, we will listen on all readers.
		readerConfig->setReaderUnit(readerConfig->getReaderProvider()->createReaderUnit());

		/*boost::shared_ptr<OmnikeyXX21ReaderUnit> readerUnit(new OmnikeyXX21ReaderUnit("OMNIKEY CardMan 5x21-CL 0"));
		readerUnit->setReaderProvider(readerConfig->getReaderProvider());
		readerConfig->setReaderUnit(readerUnit);
		boost::shared_ptr<OmnikeyXX21ReaderUnitConfiguration> readerUnitConfig(new OmnikeyXX21ReaderUnitConfiguration());
		readerUnitConfig->setUseSecureMode(false);
		readerUnit->setConfiguration(readerUnitConfig);*/

		// SmartID
		//readerConfig->setReaderProvider(SmartIDReaderProvider::getSingletonInstance());
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//boost::shared_ptr<ReaderUnit> readerUnit(new SmartIDReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerConfig->getReaderProvider());
		//readerConfig->setReaderUnit(readerUnit);
		
		// ELA
		/*readerProvider = SCIELReaderProvider::getSingletonInstance();
		boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		readerUnit.reset(new SCIELReaderUnit(serialPortXml));
		readerUnit->setReaderProvider(readerProvider);
		readerUnit->connectToReader();*/

		// Tracilock
		//readerProvider = TracilockReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new TracilockReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// Deister
		//readerProvider = DeisterReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new DeisterReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// RFIDeas
		//readerConfig->setReaderProvider(RFIDeasReaderProvider::getSingletonInstance());
		//readerConfig->setReaderUnit(readerConfig->getReaderProvider()->createReaderUnit());

		// Admitto
		//readerProvider = AdmittoReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new AdmittoReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// AxessTMC Legic
		//readerProvider = AxessTMCLegicReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new AxessTMCLegicReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// AxessTMC 13Mhz
		//readerProvider = AxessTMC13ReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new AxessTMC13ReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// Promag
		//readerProvider = PromagReaderProvider::getSingletonInstance();
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//readerUnit.reset(new PromagReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerProvider);

		// A3M LGM5600
		//readerProvider = A3MLGM5600ReaderProvider::getSingletonInstance();
		//boost::shared_ptr<A3MLGM5600ReaderUnitConfiguration> readerUnitConfig(new A3MLGM5600ReaderUnitConfiguration());
		//readerUnitConfig->setReaderIpAddress("10.2.0.103");
		//readerUnit = readerProvider->createReaderUnit();
		//readerUnit->setConfiguration(readerUnitConfig);

		// STid STR

		/*readerConfig->setReaderProvider(STidSTRReaderProvider::getSingletonInstance());
		boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		boost::shared_ptr<ReaderUnit> readerUnit(new STidSTRReaderUnit(serialPortXml));
		readerUnit->setReaderProvider(readerConfig->getReaderProvider());
		boost::shared_ptr<STidSTRReaderUnitConfiguration> readerunitConfig = boost::dynamic_pointer_cast<STidSTRReaderUnitConfiguration>(readerUnit->getConfiguration());
		readerunitConfig->setCommunicationMode(STID_CM_CIPHERED_AND_SIGNED);

		boost::shared_ptr<HMAC1Key> hmac1(new HMAC1Key(""));
		boost::shared_ptr<AES128Key> aes1(new AES128Key(""));
		readerunitConfig->setHMACKey(hmac1);
		readerunitConfig->setAESKey(aes1);

		readerConfig->setReaderUnit(readerUnit);*/

		// Elatec
		//readerConfig->setReaderProvider(ElatecReaderProvider::getSingletonInstance());
		//boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		//boost::shared_ptr<ReaderUnit> readerUnit(new ElatecReaderUnit(serialPortXml));
		//readerUnit->setReaderProvider(readerConfig->getReaderProvider());
		//readerConfig->setReaderUnit(readerUnit);

		// IdOnDemand
		/*readerConfig->setReaderProvider(IdOnDemandReaderProvider::getSingletonInstance());
		boost::shared_ptr<SerialPortXml> serialPortXml(new SerialPortXml(readerSerialPort));
		boost::shared_ptr<ReaderUnit> readerUnit(new IdOnDemandReaderUnit(serialPortXml));
		readerUnit->setReaderProvider(readerConfig->getReaderProvider());
		readerConfig->setReaderUnit(readerUnit);*/

		unsigned char data[2048];
		memset(data, 0x00, sizeof(data));				

		cout << "Waiting 15 seconds for a card insertion..." << endl;

		while (1)
		{
			readerConfig->getReaderUnit()->connectToReader();

			//readerConfig->getReaderUnit()->setCardType(CT_DESFIRE_EV1);
			cout << "Time start : " << time(NULL) << endl;
			if (readerConfig->getReaderUnit()->waitInsertion(15000))
			{
				/*boost::shared_ptr<OmnikeyXX21ReaderUnitConfiguration> readerUnitConfig = boost::dynamic_pointer_cast<OmnikeyXX21ReaderUnitConfiguration>(readerConfig->getReaderUnit()->getConfiguration());
				if (readerUnitConfig)
				{
					readerUnitConfig->setUseSecureMode(false);
					readerUnitConfig->setSecureReadKey(boost::shared_ptr<TripleDESKey>(new TripleDESKey(std::string(BE_SMART_NOT_STUPID))));
					readerConfig->getReaderUnit()->setConfiguration(readerUnitConfig);
				}*/
				
				if (readerConfig->getReaderUnit()->connect())
				{
					//boost::dynamic_pointer_cast<OmnikeyXX21ReaderUnitConfiguration>(boost::dynamic_pointer_cast<PCSCReaderUnit>(readerConfig->getReaderUnit())->getPCSCConfiguration())->setUseSecureMode(false);
					cout << "Card inserted on reader \"" << readerConfig->getReaderUnit()->getConnectedName() << "\"." << endl;

					boost::shared_ptr<Chip> chip = readerConfig->getReaderUnit()->getSingleChip();
					cout << "Card type: " << chip->getCardType() << endl;

					std::vector<unsigned char> csn = readerConfig->getReaderUnit()->getNumber(chip);
					cout << "Card Serial Number : " << BufferHelper::getHex(csn) << endl;	

					cout << "Chip list:" << endl;
					std::vector<boost::shared_ptr<Chip>> chipList = readerConfig->getReaderUnit()->getChipList();
					for(std::vector<boost::shared_ptr<Chip>>::iterator i = chipList.begin(); i != chipList.end(); ++i)
					{
						cout << "\t" << BufferHelper::getHex(readerConfig->getReaderUnit()->getNumber((*i))) << endl;
					}

					boost::shared_ptr<CardProvider> cp = chip->getCardProvider();
					boost::shared_ptr<Profile> profile = chip->getProfile();

					//JustDoIt_MifareRead(chip, cp, profile);
					
					//JustDoIt_MAD(chip, cp, profile);

					//JustDoIt_TestSerialize();

					//JustDoIt_GendarmeriePLAIN(chip, cp, profile);
					//JustDoIt_EncodeGendarmerie(chip, cp, profile);


					readerConfig->getReaderUnit()->disconnect();
				}
				else
				{
					cout << "Error: cannot connect to the card." << endl;
				}

				cout << "Logical automatic card removal in 15 seconds..." << endl;

				if (!readerConfig->getReaderUnit()->waitRemoval(15000))
				{
					cerr << "Card removal forced." << endl;
				}

				cout << "Card removed." << endl;
			} else
			{
				cout << "No card inserted." << endl;
			}
		}

	//	readerConfig->getReaderUnit()->disconnectFromReader();
	//	readerConfig->getReaderProvider()->refreshReaderList();
	}
	catch (std::exception& ex)
	{
		cout << ex.what() << endl;
	}

	return EXIT_SUCCESS;
}
