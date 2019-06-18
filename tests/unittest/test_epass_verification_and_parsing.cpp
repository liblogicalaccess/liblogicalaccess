#include "gtest/gtest.h"
#include <logicalaccess/plugins/cards/epass/epassidentitycardservice.hpp>
#include <logicalaccess/tlv.hpp>
#include <iostream>
#include <fstream>

TEST(tets_epass_verification_and_parsing, test_get_CSCA_from_masterlist)
{
  std::ifstream myfile("masterTest.ml", std::ios::binary);
  std::string stock;
  std::shared_ptr<logicalaccess::EPassIdentityCardService> srv = std::make_shared<logicalaccess::EPassIdentityCardService>(nullptr);
  std::vector<ByteVector> tmp;

  ASSERT_TRUE(myfile.is_open());
  if (myfile.is_open())
  {
    std::string s;
    while (std::getline(myfile, s))
    {
      stock += s + '\n';
    }
    myfile.close();
  }
  stock.pop_back();
  tmp = srv->extractCertificatesFromMasterList(ByteVector(stock.begin(), stock.end()));
  ASSERT_EQ(357, tmp.size());
}


TEST(tets_epass_verification_and_parsing, test_verify_false_passport_certificate_with_masterlist)
{
  std::ifstream myfile2("falseCert.cer", std::ios::binary);
  ByteVector tmp2;
  std::string stock2;
  std::shared_ptr<logicalaccess::EPassIdentityCardService> srv = std::make_shared<logicalaccess::EPassIdentityCardService>(nullptr);

  ASSERT_TRUE(myfile2.is_open());
  if (myfile2.is_open())
  {
    std::string s;
    while (std::getline(myfile2, s))
    {
      stock2 += s + '\n';
    }
    myfile2.close();
  }
  stock2.pop_back();
  tmp2 = ByteVector(stock2.begin(), stock2.end());
  std::ifstream myfile("masterTest.ml", std::ios::binary);
  ByteVector tmp;
  std::string stock;

  ASSERT_TRUE(myfile.is_open());
  if (myfile.is_open())
  {
    std::string s;
    while (std::getline(myfile, s))
    {
      stock += s + '\n';
    }
    myfile.close();
  }
  stock.pop_back();
  tmp = srv->extractCertificatesFromMasterList(ByteVector(stock.begin(), stock.end()))[188];
  tmp = srv->binaryTo64Certificate(tmp);
  ASSERT_NE(1, srv->verifyCertificate(tmp2, tmp));
}


TEST(tets_epass_verification_and_parsing, test_verify_false_passport_certificate)
{
  std::ifstream myfile2("falseCert.cer", std::ios::binary);
  ByteVector tmp2;
  std::string stock2;
  std::shared_ptr<logicalaccess::EPassIdentityCardService> srv = std::make_shared<logicalaccess::EPassIdentityCardService>(nullptr);

  ASSERT_TRUE(myfile2.is_open());
  if (myfile2.is_open())
  {
    std::string s;
    while (std::getline(myfile2, s))
    {
      stock2 += s + '\n';
    }
    myfile2.close();
  }
  stock2.pop_back();
  tmp2 = ByteVector(stock2.begin(), stock2.end());
  std::ifstream myfile("CSCA-FRANCE-2015.cer", std::ios::binary);
  ByteVector tmp;
  std::string stock;

  ASSERT_TRUE(myfile.is_open());
  if (myfile.is_open())
  {
    std::string s;
    while (std::getline(myfile, s))
    {
      stock += s + '\n';
    }
    myfile.close();
  }
  stock.pop_back();
  tmp =  ByteVector(stock.begin(), stock.end());
  ASSERT_NE(1, srv->verifyCertificate(tmp2, tmp));
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
