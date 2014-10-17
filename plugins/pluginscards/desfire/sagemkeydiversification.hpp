#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include <vector>

#ifndef SAGEMKEYDIVERSIFICATION_HPP__
#define SAGEMKEYDIVERSIFICATION_HPP__

namespace logicalaccess
{
    class LIBLOGICALACCESS_API SagemKeyDiversification : public KeyDiversification
    {
    public:
        virtual void initDiversification(std::vector<unsigned char> identifier, int AID, std::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify);
        virtual std::vector<unsigned char> getDiversifiedKey(std::shared_ptr<Key> key, std::vector<unsigned char> diversify);

        SagemKeyDiversification() {};
        ~SagemKeyDiversification() {};

        virtual std::string getType() { return "Sagem"; };

        virtual void serialize(boost::property_tree::ptree& parentNode);
        virtual void unSerialize(boost::property_tree::ptree& node);
        virtual std::string getDefaultXmlNodeName() const { return "SagemKeyDiversification"; };
    };
}

#endif