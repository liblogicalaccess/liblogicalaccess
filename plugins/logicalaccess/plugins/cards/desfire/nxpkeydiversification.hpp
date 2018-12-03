#ifndef NXPKEYDIVERSIFICATION_HPP__
#define NXPKEYDIVERSIFICATION_HPP__

#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/key.hpp>
#include <vector>
#include <string>
#include <logicalaccess/plugins/cards/desfire/lla_cards_desfire_api.hpp>

namespace logicalaccess
{
class LLA_CARDS_DESFIRE_API NXPKeyDiversification : public KeyDiversification
{
  public:
    void initDiversification(ByteVector identifier, unsigned int AID,
                             std::shared_ptr<Key> key, unsigned char keyno,
                             ByteVector &diversify) override = 0;
    ByteVector getDiversifiedKey(std::shared_ptr<Key> key,
                                 ByteVector diversify) override = 0;

    NXPKeyDiversification()
    {
    }
    virtual ~NXPKeyDiversification() = default;

    std::string getKeyDiversificationType() override
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