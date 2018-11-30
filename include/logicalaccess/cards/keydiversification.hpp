#ifndef KEYDIVERSIFICATION_HPP__
#define KEYDIVERSIFICATION_HPP__

#include <vector>
#include <memory>
#include <logicalaccess/key.hpp>

namespace logicalaccess
{
class Key;

class LIBLOGICALACCESS_API KeyDiversification : public XmlSerializable
{
  public:
    virtual ~KeyDiversification() = default;

    virtual void initDiversification(ByteVector d_identifier, unsigned int AID,
                                     std::shared_ptr<Key> key, unsigned char keyno,
                                     ByteVector &diversify) = 0;
    virtual ByteVector getDiversifiedKey(std::shared_ptr<Key> key,
                                         ByteVector diversify) = 0;
    virtual std::string getKeyDiversificationType()            = 0;

    static std::shared_ptr<KeyDiversification>
    getKeyDiversificationFromType(std::string kdiv);
};
}

#endif
