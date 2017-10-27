#ifndef NXPKEYDIVERSIFICATION_HPP__
#define NXPKEYDIVERSIFICATION_HPP__

#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include <vector>
#include <string>

namespace logicalaccess
{
class LIBLOGICALACCESS_API NXPKeyDiversification : public KeyDiversification
{
  public:
    void initDiversification(ByteVector identifier, int AID, std::shared_ptr<Key> key,
                             unsigned char keyno, ByteVector &diversify) override = 0;
    ByteVector getDiversifiedKey(std::shared_ptr<Key> key,
                                 ByteVector diversify) override = 0;

    NXPKeyDiversification()
    {
    }
    virtual ~NXPKeyDiversification() = default;

    std::string getType() override
    {
        return "NXP";
    }

    void serialize(boost::property_tree::ptree & /*parentNode*/) override
    {
    }
    void unSerialize(boost::property_tree::ptree & /*node*/) override
    {
    }
    std::string getDefaultXmlNodeName() const override
    {
        return "NXPKeyDiversification";
    }
};
}

#endif