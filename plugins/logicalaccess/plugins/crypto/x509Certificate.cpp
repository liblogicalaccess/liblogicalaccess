#include <logicalaccess/plugins/crypto/x509Certificate.hpp>

using namespace logicalaccess;

X509Certificate::X509Certificate()
{
  _certificate = nullptr;
  _bio = nullptr;
}

X509Certificate::X509Certificate(std::string data)
{
  _rawData = data;
  _bio = BIO_new(BIO_s_mem());
  BIO_puts(_bio, data.c_str());
  _certificate = PEM_read_bio_X509(_bio, NULL, NULL, NULL);
  _key = std::make_shared<PublicKey>(X509_get_pubkey(_certificate));
}

X509Certificate::~X509Certificate()
{
  freeAllData();
}

void X509Certificate::freeAllData()
{
  if (_certificate != nullptr)
    X509_free(_certificate);
  if (_bio != nullptr)
    BIO_free(_bio);
}

void X509Certificate::setData(std::string data)
{
  freeAllData();
  _rawData = data;
  BIO_puts(_bio, data.c_str());
  _certificate = PEM_read_bio_X509(_bio, NULL, NULL, NULL);
  _key->setPublicKey(X509_get_pubkey(_certificate));
}

std::string X509Certificate::getRawData() const
{
 return _rawData;
}

std::shared_ptr<PublicKey> X509Certificate::getKey()
{
  return _key;
}

int X509Certificate::verify(std::shared_ptr<PublicKey> key)
{
  return X509_verify(_certificate, _key->getPublicKey());
}
