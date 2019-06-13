#include <logicalaccess/plugins/cards/epass/epassidentitycardservice.hpp>
#include <logicalaccess/plugins/cards/epass/epassaccessinfo.hpp>
#include <logicalaccess/plugins/cards/epass/epasschip.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <memory>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/x509Certificate.hpp>
#include <logicalaccess/plugins/crypto/pkcs7Certificate.hpp>

using namespace logicalaccess;

EPassIdentityCardService::EPassIdentityCardService(const std::shared_ptr<Chip> &chip)
    : IdentityCardService(chip)
{
}

std::string EPassIdentityCardService::getName()
{
    auto dg1 = getDG1();

    auto name = dg1.owner_name_;
    // We replace '<' by space, and truncate.

    boost::algorithm::replace_all(name, "<", " ");
    boost::algorithm::trim(name);
    return name;
}

ByteVector EPassIdentityCardService::getPicture()
{
    LLA_LOG_CTX("EPassIdentityCardService::getPicture");
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityCardService");

    auto cmd = chip->getEPassCommands();
    assert(cmd);

    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    auto dg2 = cmd->readDG2();

    if (dg2.infos_.size())
        return dg2.infos_[0].image_data_;
    return {};
}

std::shared_ptr<EPassChip> EPassIdentityCardService::getEPassChip() const
{
    return std::dynamic_pointer_cast<EPassChip>(getChip());
}

std::shared_ptr<EPassAccessInfo> EPassIdentityCardService::getEPassAccessInfo() const
{
    auto ai = std::dynamic_pointer_cast<EPassAccessInfo>(access_info_);
    EXCEPTION_ASSERT_WITH_LOG(ai, LibLogicalAccessException,
                              "EPassIdentity Service expects a valid EPassAccessInfo");

    return ai;
}

EPassDG1 EPassIdentityCardService::getDG1()
{
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityCardService");

    auto cmd = chip->getEPassCommands();
    assert(cmd);

    if (dg1_cache_)
        return *dg1_cache_;
    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    dg1_cache_ = std::make_unique<EPassDG1>(cmd->readDG1());

    return *dg1_cache_;
}

ByteVector EPassIdentityCardService::getData(MetaData what)
{
    ByteVector out;
    if (what == MetaData::PICTURE)
    {
        out = getPicture();
    }

    return out;
}

int EPassIdentityCardService::verifyCertificate(ByteVector derCert, ByteVector x509Cert)
{
  int result;

  std::shared_ptr<logicalaccess::Pkcs7Certificate> p = std::make_shared<logicalaccess::Pkcs7Certificate>(std::string(derCert.begin(), derCert.end()));
  std::shared_ptr<logicalaccess::X509Certificate> px = std::make_shared<logicalaccess::X509Certificate>(std::string(x509Cert.begin(), x509Cert.end()));
  result = p->verify(px->getKey());
  return result;
}

int EPassIdentityCardService::verifyCertificate(ByteVector x509Cert)
{
  auto chip = getEPassChip();
  EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                            "No or invalid chip object in EPassIdentityCardService");

  auto srv = std::dynamic_pointer_cast<logicalaccess::EPassIdentityCardService>(chip->getService(logicalaccess::CST_IDENTITY));
  ByteVector cds = srv->getCertificate();
  return verifyCertificate(cds, x509Cert);
}

std::vector<ByteVector> EPassIdentityCardService::getCSCACertificatesFromMasterlist(std::string path)
{
  std::ifstream myfile(path, std::ios::binary);
  std::string stock;

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
  ByteVector vec;
  vec.insert(vec.begin(), stock.begin(), stock.end());
  return extractCertificatesFromMasterList(vec);
}

std::vector<ByteVector> EPassIdentityCardService::extractCertificatesFromMasterList(const ByteVector &bytes)
{
  std::vector<ByteVector> certificates;
  std::vector<TLVPtr> tmp;
  size_t i = 0;

  /*
  **  The TLV tags of the masterlists doesn't help us at findig certficates but
  **  the location of the certifcates is always the same in all masterlists so
  **  we can search it manually;
  */

  tmp = TLV::parse_tlvs(bytes, i);
  tmp = TLV::parse_tlvs(tmp[0]->value(), i);
  tmp = TLV::parse_tlvs(tmp[1]->value(), i);
  tmp = TLV::parse_tlvs(tmp[0]->value(), i);
  tmp = TLV::parse_tlvs(tmp[2]->value(), i);
  tmp = TLV::parse_tlvs(tmp[1]->value(), i);
  tmp = TLV::parse_tlvs(tmp[0]->value(), i);
  tmp = TLV::parse_tlvs(tmp[0]->value(), i);
  tmp = TLV::parse_tlvs(tmp[1]->value(), i);
  for (size_t j = 0; j != tmp.size(); j++)
    certificates.push_back(tmp[j]->getCompletTLV());
  return certificates;
}

ByteVector  EPassIdentityCardService::binaryTo64Certificate(ByteVector derCert)
{
  std::string str("-----BEGIN CERTIFICATE-----\n");
  static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t i = 0;
  size_t ix = 0;
  size_t leng = derCert.size();
  std::stringstream q;

  for(i = 0, ix = leng - leng % 3; i < ix; i += 3)
  {
      q<< base64_chars[ (derCert[i] & 0xfc) >> 2 ];
      q<< base64_chars[ ((derCert[i] & 0x03) << 4) + ((derCert[i + 1] & 0xf0) >> 4)  ];
      q<< base64_chars[ ((derCert[i + 1] & 0x0f) << 2) + ((derCert[i + 2] & 0xc0) >> 6)  ];
      q<< base64_chars[ derCert[i + 2] & 0x3f ];
  }
  if (ix < leng)
  {
    q<< base64_chars[ (derCert[ix] & 0xfc) >> 2 ];
    q<< base64_chars[ ((derCert[ix] & 0x03) << 4) + (ix + 1 < leng ? (derCert[ix + 1] & 0xf0) >> 4 : 0)];
    q<< (ix + 1 < leng ? base64_chars[ ((derCert[ix + 1] & 0x0f) << 2) ] : '=');
    q<< '=';
  }
  str += q.str();
  int l = 1;
  std::string base64;
  for (int j = 0; j != str.size(); j++)
  {
    base64 += str[j];
    if (l == 64)
    {
      base64+='\n';
      l = 0;
    }
    l++;
  }
  if (l != 0 && base64[base64.size() - 1] != '\n')
    base64 += '\n';
  base64 += "-----END CERTIFICATE-----\n";
  return ByteVector(base64.begin(), base64.end());
}

int EPassIdentityCardService::verifyCertificateWithMasterList(ByteVector derCert, std::string path)
{
  int ret = 0;
  std::vector<ByteVector> CSCAList = getCSCACertificatesFromMasterlist(path);

  for (size_t i = 0; i != CSCAList.size() && ret != 1; i++)
  {
    ByteVector vec = binaryTo64Certificate(CSCAList[i]);
    ret = verifyCertificate(vec);
  }
  return ret;
}

int EPassIdentityCardService::verifyCertificateWithMasterList(std::string path)
{
  auto chip = getEPassChip();
  EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                            "No or invalid chip object in EPassIdentityCardService");

  auto srv = std::dynamic_pointer_cast<logicalaccess::EPassIdentityCardService>(chip->getService(logicalaccess::CST_IDENTITY));
  ByteVector cds = srv->getCertificate();
  return verifyCertificateWithMasterList(cds, path);
}

ByteVector EPassIdentityCardService::getCertificate()
{;
  ByteVector stock;

  auto chip = getEPassChip();
  EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                            "No or invalid chip object in EPassIdentityCardService");

  auto cmd = chip->getEPassCommands();
  assert(cmd);
  stock = cmd->readSOD();
  stock.erase(stock.begin(), stock.begin() + 4);
  return stock;
}

std::string EPassIdentityCardService::getString(MetaData what)
{
    std::string out;
    if (what == MetaData::NAME)
    {
        out = getName();
    }
    else if (what == MetaData::NATIONALITY)
    {
        out = getDG1().nationality_;
    }
    else if (what == MetaData::DOC_NO)
    {
        out = getDG1().doc_no_;
    }
    return out;
}

std::chrono::system_clock::time_point EPassIdentityCardService::getTime(MetaData what)
{
    std::chrono::system_clock::time_point out;
    if (what == MetaData::BIRTHDATE)
    {
        out = getDG1().birthdate_;
    }
    else if (what == MetaData::EXPIRATION)
    {
        out = getDG1().expiration_;
    }
    return out;
}
