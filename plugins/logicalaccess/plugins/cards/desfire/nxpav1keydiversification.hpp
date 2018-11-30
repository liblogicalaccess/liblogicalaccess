#ifndef NXPAV1KEYDIVERSIFICATION_HPP__
#define NXPAV1KEYDIVERSIFICATION_HPP__

#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/desfire/nxpkeydiversification.hpp>
#include <vector>
#include <string>

namespace logicalaccess
{
class LIBLOGICALACCESS_API NXPAV1KeyDiversification : public NXPKeyDiversification
{
  public:
    void initDiversification(ByteVector identifier, unsigned int AID,
                             std::shared_ptr<Key> key, unsigned char keyno,
                             ByteVector &diversify) override;
    ByteVector getDiversifiedKey(std::shared_ptr<Key> key, ByteVector diversify) override;

    NXPAV1KeyDiversification()
    {
    }
    virtual ~NXPAV1KeyDiversification()
    {
    }

    std::string getKeyDiversificationType() override
    {
        return "NXPAV1";
    }

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &node) override;
    std::string getDefaultXmlNodeName() const override
    {
        return "NXPAV1KeyDiversification";
    }
};
}

#endif