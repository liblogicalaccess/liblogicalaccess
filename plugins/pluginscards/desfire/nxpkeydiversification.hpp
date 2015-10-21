#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include <vector>

#ifndef NXPKEYDIVERSIFICATION_HPP__
#define NXPKEYDIVERSIFICATION_HPP__

#include <string>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API NXPKeyDiversification : public KeyDiversification
    {
    public:
        virtual void initDiversification(std::vector<unsigned char> identifier, int AID, std::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify) = 0;
        virtual std::vector<unsigned char> getDiversifiedKey(std::shared_ptr<Key> key, std::vector<unsigned char> diversify) = 0;

        NXPKeyDiversification() {};
            virtual ~NXPKeyDiversification() = default;

        virtual std::string getType() { return "NXP"; };

        virtual void serialize(boost::property_tree::ptree& parentNode) {};
        virtual void unSerialize(boost::property_tree::ptree& node) {};
        virtual std::string getDefaultXmlNodeName() const { return "NXPKeyDiversification"; };

    private:
    };
}

#endif