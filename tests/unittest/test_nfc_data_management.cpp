#include "gtest/gtest.h"
#include <logicalaccess/services/nfctag/nfcdata.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/services/nfctag/lockcontroltlv.hpp>
#include <logicalaccess/services/nfctag/memorycontroltlv.hpp>

TEST(test_nfc_data, test_ndef_to_tlv)
{
  std::vector<uint8_t> testVec = {0xD1, 0x01, 0x04, 0x01, 'T', 'e', 's', 't'};
  std::shared_ptr<logicalaccess::NdefMessage> ndefMes = std::make_shared<logicalaccess::NdefMessage>(testVec);
  std::vector<uint8_t> resVec = logicalaccess::NfcData::dataToTLV(ndefMes);
  std::vector<uint8_t> tmpVec(resVec.begin() + 2, resVec.end() - 1);

  ASSERT_EQ(0x03, resVec[0]);
  ASSERT_EQ(testVec.size(), resVec[1]);
  ASSERT_EQ(testVec, tmpVec);
}

TEST(test_nfc_data, test_tlv_to_ndef)
{
  std::vector<uint8_t> testTLV = {0x03, 0x08 ,0xD1, 0x01, 0x04, 0x01, 'T', 'e', 's', 't', 0xFE};
  std::vector<uint8_t> tmpVec(testTLV.begin() + 6, testTLV.end() - 1);
  std::shared_ptr<logicalaccess::NdefMessage> ndefMes;
  ndefMes = std::dynamic_pointer_cast<logicalaccess::NdefMessage>(logicalaccess::NfcData::tlvToData(testTLV)[0]);
  std::vector<uint8_t> record;

  ASSERT_TRUE(ndefMes != nullptr);
  record = ndefMes->getRecords()[0]->getPayload();
  ASSERT_EQ(tmpVec, record);
}

TEST(test_nfc_data, test_lockcontrol_to_tlv)
{
  std::shared_ptr<logicalaccess::LockControlTlv> lock = std::make_shared<logicalaccess::LockControlTlv>(0x60);
  std::vector<uint8_t> resVec = logicalaccess::NfcData::dataToTLV(lock);

  ASSERT_EQ(resVec.size(), 0x05);
  ASSERT_EQ(0xA0, resVec[2]);
  ASSERT_EQ(lock->getNumberOfDynamicLockBits(), resVec[3]);
  ASSERT_EQ(0x44, resVec[4]);
}

TEST(test_nfc_data, test_tlv_to_lockcontrol)
{
  std::vector<uint8_t> tlv = {0x01, 0x03, 0xA0, 0x10, 0x44};
  std::shared_ptr<logicalaccess::LockControlTlv> lock = std::dynamic_pointer_cast<logicalaccess::LockControlTlv>(logicalaccess::NfcData::tlvToData(tlv)[0]);
  unsigned char bpp = tlv[4] >> 4;
  unsigned char blplb = tlv[4] << 4;

  blplb = blplb >> 4;
  ASSERT_FALSE(lock == nullptr);
  ASSERT_EQ(tlv[2] - 64, lock->getAdditionalDataSize());
  ASSERT_EQ(tlv[3], lock->getNumberOfDynamicLockBits());
  ASSERT_EQ(bpp, lock->getBytesPerPage());
  ASSERT_EQ(blplb,lock->getBytesLockedPerLockBit());
}

TEST(test_nfc_data, test_memorycontrol_to_tlv)
{
  std::shared_ptr<logicalaccess::MemoryControlTlv> mem = std::make_shared<logicalaccess::MemoryControlTlv>(0x8E, 0x04);
  std::vector<uint8_t> resVec = logicalaccess::NfcData::dataToTLV(mem);

  ASSERT_EQ(resVec.size(), 0x05);
  ASSERT_EQ(0x8E, resVec[2]);
  ASSERT_EQ(0x04, resVec[3]);
  ASSERT_EQ(0x40, resVec[4]);
}

TEST(test_nfc_data, test_tlv_to_memorycontrol)
{
  std::vector<uint8_t> tlv = {0x02, 0x03, 0x8E, 0x04, 0x40};
  std::shared_ptr<logicalaccess::MemoryControlTlv> mem = std::dynamic_pointer_cast<logicalaccess::MemoryControlTlv>(logicalaccess::NfcData::tlvToData(tlv)[0]);

  ASSERT_FALSE(mem == nullptr);
  ASSERT_EQ(tlv[2], mem->getByteAddr());
  ASSERT_EQ(tlv[3], mem->getSize());
  ASSERT_EQ(tlv[4] >> 4, mem->getBytesPerPage());
}

TEST(test_nfc_data, test_multiple_ndef_messages)
{
  std::vector<uint8_t> testVec = {0xD1, 0x01, 0x04, 0x01, 'T', 'e', 's', 't'};
  std::vector<uint8_t> memoryVec;
  std::vector<uint8_t> ndefVec;
  std::vector<std::shared_ptr<logicalaccess::NfcData>> v;
  std::shared_ptr<logicalaccess::NdefMessage> ndefMes = std::make_shared<logicalaccess::NdefMessage>(testVec);
  std::shared_ptr<logicalaccess::LockControlTlv> lock = std::make_shared<logicalaccess::LockControlTlv>(0x60);
  std::shared_ptr<logicalaccess::MemoryControlTlv> mem = std::make_shared<logicalaccess::MemoryControlTlv>(0xE8, 0x04);

  testVec = logicalaccess::NfcData::dataToTLV(lock);
  memoryVec = logicalaccess::NfcData::dataToTLV(mem);
  ndefVec = logicalaccess::NfcData::dataToTLV(ndefMes);
  testVec.insert(testVec.end(), memoryVec.begin(), memoryVec.end());
  testVec.insert(testVec.end(), ndefVec.begin(), ndefVec.end());
  v = logicalaccess::NfcData::tlvToData(testVec);

  ASSERT_FALSE(v.size() != 3 );
  ASSERT_EQ(0x01, v[0]->getType());
  ASSERT_EQ(0x02, v[1]->getType());
  ASSERT_EQ(0x03, v[2]->getType());
}
