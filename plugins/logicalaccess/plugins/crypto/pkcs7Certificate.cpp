#include <logicalaccess/plugins/crypto/pkcs7Certificate.hpp>

using namespace logicalaccess;

Pkcs7Certificate::Pkcs7Certificate()
{
  _certificate = nullptr;
}

Pkcs7Certificate::Pkcs7Certificate(std::string data)
{
  _rawData = data;
  const unsigned char *tmp = reinterpret_cast<const unsigned char*>(data.c_str());
  _certificate =  d2i_PKCS7(nullptr, &tmp, data.size());
}

Pkcs7Certificate::~Pkcs7Certificate()
{
  if (_certificate != nullptr)
    PKCS7_free(_certificate);
}

void Pkcs7Certificate::setData(std::string data)
{
  if (_certificate != nullptr)
    PKCS7_free(_certificate);
  _rawData = data;
  const unsigned char *tmp = reinterpret_cast<const unsigned char*>(data.c_str());
  _certificate =  d2i_PKCS7(nullptr, &tmp, data.size());
}

std::string Pkcs7Certificate::getRawData() const
{
  return _rawData;
}

int Pkcs7Certificate::verify(std::shared_ptr<PublicKey> key)
{
  X509 *cert = sk_X509_value(_certificate->d.sign->cert, 0);
  return X509_verify(cert, key->getPublicKey());
}

std::string Pkcs7Certificate::getExpiryDate()
{
  std::string date;

  if (_certificate != nullptr)
  {
    X509 *cert = sk_X509_value(_certificate->d.sign->cert, 0);
    date = std::string(reinterpret_cast<const char*>(cert->cert_info->validity->notAfter->data));
  }
  return date;
}
