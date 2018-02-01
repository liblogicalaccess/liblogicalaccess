#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/readerproviders/readerconfiguration.hpp"
#include "logicalaccess/services/storage/storagecardservice.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"
#include "logicalaccess/readerproviders/serialportdatatransport.hpp"
#include "logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp"
#include "logicalaccess/plugins/lla-tests/macros.hpp"
#include "logicalaccess/plugins/lla-tests/utils.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityrightparity2format.hpp"
#include "logicalaccess/services/accesscontrol/formats/asciiformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/bariumferritepcscformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand35format.hpp"
#include "logicalaccess/services/accesscontrol/formats/dataclockformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/getronik40bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/hidhoneywell40bitformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/rawformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/paritydatafield.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp"
#include "logicalaccess/services/accesscontrol/encodings/littleendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp"
#include <gtest/gtest.h>

using namespace logicalaccess;

TEST(test_format_utils, test_format_wiegand26)
{
    auto formatWiegand26 = std::make_shared<Wiegand26Format>();
    formatWiegand26->setUid(1000);
    formatWiegand26->setFacilityCode(67);
    std::vector<unsigned char> formatBuf = formatWiegand26->getLinearData();
    auto result = std::vector<unsigned char>({0xa1, 0x81, 0xf4, 0x40});
    ASSERT_EQ(formatBuf, result);
    formatWiegand26 = std::make_shared<Wiegand26Format>();
    formatWiegand26->setLinearData(result);
    ASSERT_EQ(formatWiegand26->getUid(), 1000);
    ASSERT_EQ(formatWiegand26->getFacilityCode(), 67);
}

TEST(test_format_utils, test_format_wiegand37)
{
    auto formatWiegand37 = std::make_shared<Wiegand37Format>();
    formatWiegand37->setUid(98765);
    auto formatBuf = formatWiegand37->getLinearData();
    auto result    = std::vector<unsigned char>({0x00, 0x00, 0x18, 0x1c, 0xd8});
    ASSERT_EQ(formatBuf, result);
    formatWiegand37 = std::make_shared<Wiegand37Format>();
    formatWiegand37->setLinearData(result);
    ASSERT_EQ(formatWiegand37->getUid(), 98765);
}

TEST(test_format_utils, test_format_wiegand34)
{
    auto formatWiegand34 = std::make_shared<Wiegand34Format>();
    formatWiegand34->setUid(9865);
    auto formatBuf = formatWiegand34->getLinearData();
    auto result    = std::vector<unsigned char>({0x00, 0x00, 0x13, 0x44, 0xc0});
    ASSERT_EQ(formatBuf, result);
    formatWiegand34 = std::make_shared<Wiegand34Format>();
    formatWiegand34->setLinearData(result);
    ASSERT_EQ(formatWiegand34->getUid(), 9865);
}

TEST(test_format_utils, test_format_wiegand34WithFacilityFormat)
{
    auto formatWiegand34F = std::make_shared<Wiegand34WithFacilityFormat>();
    formatWiegand34F->setUid(9765);
    formatWiegand34F->setFacilityCode(89);
    auto formatBuf = formatWiegand34F->getLinearData();
    auto result    = std::vector<unsigned char>({0x00, 0x2c, 0x93, 0x12, 0xc0});
    ASSERT_EQ(formatBuf, result);
    formatWiegand34F = std::make_shared<Wiegand34WithFacilityFormat>();
    formatWiegand34F->setLinearData(result);
    ASSERT_EQ(formatWiegand34F->getUid(), 9765);
    ASSERT_EQ(formatWiegand34F->getFacilityCode(), 89);
}

TEST(test_format_utils, test_format_wiegand37WithFacilityFormat)
{

    auto formatWiegand37F = std::make_shared<Wiegand37WithFacilityFormat>();
    formatWiegand37F->setUid(95265);
    formatWiegand37F->setFacilityCode(15);
    auto formatBuf = formatWiegand37F->getLinearData();
    auto result    = std::vector<unsigned char>({0x00, 0x07, 0x97, 0x42, 0x10});
    ASSERT_EQ(formatBuf, result);
    formatWiegand37F = std::make_shared<Wiegand37WithFacilityFormat>();
    formatWiegand37F->setLinearData(result);
    ASSERT_EQ(formatWiegand37F->getUid(), 95265);
    ASSERT_EQ(formatWiegand37F->getFacilityCode(), 15);
}

TEST(test_format_utils, test_format_wiegand37WithFacilityRightParity2Format)
{

    auto formatWiegand37F2F = std::make_shared<Wiegand37WithFacilityRightParity2Format>();
    formatWiegand37F2F->setUid(75265);
    formatWiegand37F2F->setFacilityCode(85);
    auto formatBuf = formatWiegand37F2F->getLinearData();
    auto result    = std::vector<unsigned char>({0x80, 0x2a, 0xa4, 0xc0, 0x30});
    ASSERT_EQ(formatBuf, result);
    formatWiegand37F2F = std::make_shared<Wiegand37WithFacilityRightParity2Format>();
    formatWiegand37F2F->setLinearData(result);
    ASSERT_EQ(formatWiegand37F2F->getUid(), 75265);
    ASSERT_EQ(formatWiegand37F2F->getFacilityCode(), 85);
}


TEST(test_format_utils, test_format_ASCIIFormat)
{

    auto formatASCII    = std::make_shared<ASCIIFormat>();
    std::string message = "I am locking at you from a RFID distance !";
    formatASCII->setASCIIValue(message);
    formatASCII->setASCIILength(message.size() + 10);
    auto formatBuf = formatASCII->getLinearData();
    auto result    = std::vector<unsigned char>(message.begin(), message.end());
    for (int x = 0; x < 10; ++x)
        result.push_back(' ');
    ASSERT_EQ(formatBuf, result);
    formatASCII = std::make_shared<ASCIIFormat>();
    formatASCII->setASCIILength(message.size() + 10);
    formatASCII->setLinearData(result);
    ASSERT_EQ(formatASCII->getASCIIValue(), message);
}

TEST(test_format_utils, test_format_Wiegand35format)
{
    auto formatWiegand35 = std::make_shared<Wiegand35Format>();
    formatWiegand35->setUid(39248);
    formatWiegand35->setCompanyCode(235);
    auto formatBuf = formatWiegand35->getLinearData();
    auto result    = std::vector<unsigned char>({0xc3, 0xac, 0x26, 0x54, 0x20});
    ASSERT_EQ(formatBuf, result);
    formatWiegand35 = std::make_shared<Wiegand35Format>();
    formatWiegand35->setLinearData(result);
    ASSERT_EQ(formatWiegand35->getUid(), 39248);
    ASSERT_EQ(formatWiegand35->getCompanyCode(), 235);
}

TEST(test_format_utils, test_format_DataClockFormat)
{
    auto formatDataClock = std::make_shared<DataClockFormat>();
    formatDataClock->setUid(974641);
    auto formatBuf = formatDataClock->getLinearData();
    auto result    = std::vector<unsigned char>({0x00, 0x97, 0x46, 0x41, 0xd0});
    ASSERT_EQ(formatBuf, result);
    formatDataClock = std::make_shared<DataClockFormat>();
    formatDataClock->setLinearData(result);
    ASSERT_EQ(formatDataClock->getUid(), 974641);
}

TEST(test_format_utils, test_format_RawFormat)
{
    auto formatRaw = std::make_shared<RawFormat>();
    auto rawData = ByteVector({0x89, 0x59, 0x48, 0x48, 0x56, 0x48, 0x35, 0x48, 0x78, 0x99,
                               0x15, 0x54, 0x36, 0x75, 0x12});
    formatRaw->setRawData(rawData);
    auto formatBuf = formatRaw->getLinearData();
    ASSERT_EQ(formatBuf, rawData);
    formatRaw   = std::make_shared<RawFormat>();
    auto result = rawData;
    formatRaw->setLinearData(result);
    auto resultRawData = formatRaw->getRawData();
    ASSERT_EQ(resultRawData, rawData);
}

TEST(test_format_utils, test_format_BariumFerritePCSCFormat)
{
    auto formatBariumFerritePCSC = std::make_shared<BariumFerritePCSCFormat>();
    formatBariumFerritePCSC->setUid(46417);
    formatBariumFerritePCSC->setFacilityCode(156);
    auto formatBuf = formatBariumFerritePCSC->getLinearData();
    auto result    = std::vector<unsigned char>({0xf0, 0x9c, 0xb5, 0x51, 0x88});
    ASSERT_EQ(formatBuf, result);
    formatBariumFerritePCSC = std::make_shared<BariumFerritePCSCFormat>();
    formatBariumFerritePCSC->setLinearData(result);
    ASSERT_EQ(formatBariumFerritePCSC->getUid(), 46417);
    ASSERT_EQ(formatBariumFerritePCSC->getFacilityCode(), 156);
}

TEST(test_format_utils, test_format_Getronik40BitFormat)
{
    auto formatGetronik40 = std::make_shared<Getronik40BitFormat>();
    formatGetronik40->setUid(5671);
    formatGetronik40->setField(5671);
    auto formatBuf = formatGetronik40->getLinearData();
    auto result    = std::vector<unsigned char>({0x2e, 0x16, 0x27, 0x58, 0x9f});
    ASSERT_EQ(formatBuf, result);
    formatGetronik40 = std::make_shared<Getronik40BitFormat>();
    formatGetronik40->setLinearData(result);
    ASSERT_EQ(formatGetronik40->getUid(), 5671);
    ASSERT_EQ(formatGetronik40->getField(), 5671);
}

TEST(test_format_utils, test_format_HIDHoneywell40Bit)
{

    auto formatHIDHoneywell40Bit = std::make_shared<HIDHoneywell40BitFormat>();
    formatHIDHoneywell40Bit->setUid(17866);
    formatHIDHoneywell40Bit->setFacilityCode(895);
    auto formatBuf = formatHIDHoneywell40Bit->getLinearData();
    auto result    = std::vector<unsigned char>({0xf3, 0x7f, 0x45, 0xca, 0x03});
    ASSERT_EQ(formatBuf, result);
    formatHIDHoneywell40Bit = std::make_shared<HIDHoneywell40BitFormat>();
    formatHIDHoneywell40Bit->setLinearData(result);
    ASSERT_EQ(formatHIDHoneywell40Bit->getUid(), 17866);
    ASSERT_EQ(formatHIDHoneywell40Bit->getFacilityCode(), 895);
}

TEST(test_format_utils, test_format_FASCN200BitFormat)
{

    // Currently not working because we use BCDNibbleDataType (4bits)
    // with parity offset 4 when FASCN200 use parity at bit 5 (outside the BCDN)
    auto formatFASCN200 = std::make_shared<FASCN200BitFormat>();
    formatFASCN200->setAgencyCode(32);
    formatFASCN200->setSystemCode(1);
    formatFASCN200->setUid(92446);
    formatFASCN200->setSerieCode(0);
    formatFASCN200->setCredentialCode(1);
    formatFASCN200->setPersonIdentifier(1112223333);
    formatFASCN200->setOrganizationalCategory(OC_FEDERAL_GOVERNMENT_AGENCY);
    formatFASCN200->setOrganizationalIdentifier(1223);
    formatFASCN200->setPOACategory(POA_CIVIL);
    auto formatBuf = formatFASCN200->getLinearData();
    auto result    = std::vector<unsigned char>(
        {0xD0, 0x43, 0x94, 0x58, 0x21, 0x0C, 0x2C, 0x19, 0xA0, 0x84, 0x6D, 0x83, 0x68,
         0x5A, 0x10, 0x82, 0x10, 0x8C, 0xE7, 0x39, 0x84, 0x10, 0x8C, 0xA3, 0xF5});
    /*
     * We currently calcul the wrong LRC.... have to found how to do it
     * if (!std::equal(formatBuf.begin(), formatBuf.end(), result.begin()))
         THROW_EXCEPTION_WITH_LOG(std::runtime_error, "Bad format");*/
    formatFASCN200 = std::make_shared<FASCN200BitFormat>();
    formatFASCN200->setLinearData(result);
    ASSERT_EQ(formatFASCN200->getAgencyCode(), 32);
    ASSERT_EQ(formatFASCN200->getSystemCode(), 1);
    ASSERT_EQ(formatFASCN200->getUid(), 92446);
    ASSERT_EQ(formatFASCN200->getSerieCode(), 0);
    ASSERT_EQ(formatFASCN200->getCredentialCode(), 1);
    ASSERT_EQ(formatFASCN200->getPersonIdentifier(), 1112223333);
    ASSERT_EQ(formatFASCN200->getOrganizationalCategory(), OC_FEDERAL_GOVERNMENT_AGENCY);
    ASSERT_EQ(formatFASCN200->getOrganizationalIdentifier(), 1223);
    ASSERT_EQ(formatFASCN200->getPOACategory(), POA_CIVIL);
}

TEST(test_format_utils, test_format_CustomFormat)
{
    auto formatCustom = std::make_shared<CustomFormat>();
    std::list<std::shared_ptr<DataField>> fieldList;

    auto numberDataField = std::make_shared<NumberDataField>();
    numberDataField->setPosition(0);
    numberDataField->setDataLength(64);
    numberDataField->setValue(984654321);
    fieldList.push_back(numberDataField);

    auto binaryDataField = std::make_shared<BinaryDataField>();
    binaryDataField->setPosition(64);
    binaryDataField->setDataLength(160);
    binaryDataField->setPaddingChar(0xFF);
    binaryDataField->setValue({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0x12, 0x34,
                               0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef});
    fieldList.push_back(binaryDataField);

    auto stringDataField = std::make_shared<StringDataField>();
    stringDataField->setPosition(224);
    stringDataField->setPaddingChar('H');
    stringDataField->setDataLength(200);
    stringDataField->setValue("Le lamasticot sur kappou !");
    fieldList.push_back(stringDataField);

    auto parityDataField = std::make_shared<ParityDataField>();
    parityDataField->setPosition(424);
    parityDataField->setBitsUsePositions({0x01, 0x08, 0x0a, 0x0c, 0x12});
    parityDataField->setParityType(PT_ODD);
    fieldList.push_back(parityDataField);

    std::shared_ptr<DataRepresentation> littleEdian =
        std::make_shared<LittleEndianDataRepresentation>();

    auto numberLittleDataField = std::make_shared<NumberDataField>();
    numberLittleDataField->setDataRepresentation(littleEdian);
    numberLittleDataField->setPosition(425);
    numberLittleDataField->setDataLength(64);
    numberLittleDataField->setValue(984654321);
    fieldList.push_back(numberLittleDataField);

    auto binaryLittleDataField = std::make_shared<BinaryDataField>();
    binaryLittleDataField->setDataRepresentation(littleEdian);
    binaryLittleDataField->setPosition(489);
    binaryLittleDataField->setDataLength(160);
    binaryLittleDataField->setPaddingChar(0xFF);
    binaryLittleDataField->setValue({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0x12,
                                     0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef});
    fieldList.push_back(binaryLittleDataField);

    auto stringLittleDataField = std::make_shared<StringDataField>();
    stringLittleDataField->setDataRepresentation(littleEdian);
    stringLittleDataField->setPosition(649);
    stringLittleDataField->setPaddingChar('H');
    stringLittleDataField->setDataLength(200);
    stringLittleDataField->setValue("Le lamasticot sur kappou !");
    fieldList.push_back(stringLittleDataField);

    auto parityLittleDataField = std::make_shared<ParityDataField>();
    parityLittleDataField->setPosition(850);
    parityLittleDataField->setBitsUsePositions({501, 356, 286, 1, 623});
    parityLittleDataField->setParityType(PT_ODD);
    fieldList.push_back(parityLittleDataField);

    std::shared_ptr<DataRepresentation> nodatarepre =
        std::make_shared<NoDataRepresentation>();

    auto numberNoDataField = std::make_shared<NumberDataField>();
    numberNoDataField->setDataRepresentation(nodatarepre);
    numberNoDataField->setPosition(851);
    numberNoDataField->setDataLength(64);
    numberNoDataField->setValue(984654321);
    fieldList.push_back(numberNoDataField);

    auto binaryNoDataField = std::make_shared<BinaryDataField>();
    binaryNoDataField->setDataRepresentation(nodatarepre);
    binaryNoDataField->setPosition(915);
    binaryNoDataField->setDataLength(160);
    binaryNoDataField->setPaddingChar(0xFF);
    binaryNoDataField->setValue({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0x12,
                                 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef});
    fieldList.push_back(binaryNoDataField);

    auto stringNoDataField = std::make_shared<StringDataField>();
    stringNoDataField->setDataRepresentation(nodatarepre);
    stringNoDataField->setPosition(1075);
    stringNoDataField->setPaddingChar('H');
    stringNoDataField->setDataLength(200);
    stringNoDataField->setValue("Le lamasticot sur kappou !");
    fieldList.push_back(stringNoDataField);

    auto parityNoDataField = std::make_shared<ParityDataField>();
    parityNoDataField->setPosition(1275);
    parityNoDataField->setBitsUsePositions({598, 123, 452, 21, 900});
    parityNoDataField->setParityType(PT_ODD);
    fieldList.push_back(parityNoDataField);

    formatCustom->setFieldList(fieldList);
    auto formatBuf = formatCustom->getLinearData();
    auto result    = ByteVector(
        {0x00, 0x00, 0x00, 0x00, 0x3a, 0xb0, 0xa1, 0xf1, 0x12, 0x34, 0x56, 0x78, 0x9a,
         0xbc, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0xff, 0xff,
         0xff, 0xff, 0x4c, 0x65, 0x20, 0x6c, 0x61, 0x6d, 0x61, 0x73, 0x74, 0x69, 0x63,
         0x6f, 0x74, 0x20, 0x73, 0x75, 0x72, 0x20, 0x6b, 0x61, 0x70, 0x70, 0x6f, 0x75,
         0x20, 0xf8, 0xd0, 0xd8, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff,
         0xf7, 0xe6, 0xde, 0x4d, 0x3c, 0x2b, 0x1a, 0x09, 0x77, 0xe6, 0xde, 0x4d, 0x3c,
         0x2b, 0x1a, 0x09, 0x10, 0x3a, 0xb7, 0xb8, 0x38, 0x30, 0xb5, 0x90, 0x39, 0x3a,
         0xb9, 0x90, 0x3a, 0x37, 0xb1, 0xb4, 0xba, 0x39, 0xb0, 0xb6, 0xb0, 0xb6, 0x10,
         0x32, 0xa6, 0x3e, 0x34, 0x36, 0x07, 0x40, 0x00, 0x00, 0x00, 0x02, 0x46, 0x8a,
         0xcf, 0x13, 0x57, 0x99, 0xbd, 0xe2, 0x46, 0x8a, 0xcf, 0x13, 0x57, 0x99, 0xbd,
         0xff, 0xff, 0xff, 0xff, 0xe9, 0x8c, 0xa4, 0x0d, 0x8c, 0x2d, 0xac, 0x2e, 0x6e,
         0x8d, 0x2c, 0x6d, 0xee, 0x84, 0x0e, 0x6e, 0xae, 0x44, 0x0d, 0x6c, 0x2e, 0x0e,
         0x0d, 0xee, 0xa4, 0x10});
    ASSERT_EQ(formatBuf, result);
    formatCustom = std::make_shared<CustomFormat>();
    formatCustom->setFieldList(fieldList);
    formatCustom->setLinearData(result);

    auto numberFieldCheck =
        std::dynamic_pointer_cast<NumberDataField>(formatCustom->getFieldForPosition(0));
    ASSERT_EQ(numberFieldCheck->getValue(), 984654321);

    auto stringFieldCheck = std::dynamic_pointer_cast<StringDataField>(
        formatCustom->getFieldForPosition(224));
    ASSERT_EQ(stringFieldCheck->getValue(), "Le lamasticot sur kappou ");

    auto binaryFieldCheck = std::dynamic_pointer_cast<BinaryDataField>(
        formatCustom->getFieldForPosition(489));
    auto value = binaryFieldCheck->getValue();
    auto rvalue =
        ByteVector({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0x12, 0x34,
                    0x56, 0x78, 0x9a, 0xbc, 0xcd, 0xef, 0xff, 0xff, 0xff, 0xff});
    ASSERT_EQ(value, rvalue);
}
