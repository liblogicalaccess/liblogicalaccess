#ifndef SAGEMKEYDIVERSIFICATION_HPP__
#define SAGEMKEYDIVERSIFICATION_HPP__

#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/key.hpp>
#include <vector>

namespace logicalaccess
{
class LIBLOGICALACCESS_API SagemKeyDiversification : public KeyDiversification
{
  public:
    void initDiversification(ByteVector identifier, unsigned int AID,
                             std::shared_ptr<Key> key, unsigned char keyno,
                             ByteVector &diversify) override;
    ByteVector getDiversifiedKey(std::shared_ptr<Key> key, ByteVector diversify) override;

    SagemKeyDiversification()
    {
    }
    ~SagemKeyDiversification()
    {
    }

    std::string getType() override
    {
        return "Sagem";
    }

    void serialize(boost::property_tree::ptree &parentNode) override;
    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override
    {
        return "SagemKeyDiversification";
    }
};
}

#endif