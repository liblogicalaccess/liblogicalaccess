#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include "nxpkeydiversification.hpp"
#include <vector>

#ifndef NXPAV2KEYDIVERSIFICATION_HPP__
#define NXPAV2KEYDIVERSIFICATION_HPP__

#include <string>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API NXPAV2KeyDiversification : public NXPKeyDiversification
    {
    public:
        virtual void initDiversification(std::vector<unsigned char> identifier, int AID, std::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify);
        virtual std::vector<unsigned char> getDiversifiedKey(std::shared_ptr<Key> key, std::vector<unsigned char> diversify);

        NXPAV2KeyDiversification() {};
        NXPAV2KeyDiversification(const std::vector<unsigned char>& divInput) : d_divInput(divInput) {};
        ~NXPAV2KeyDiversification() {};

        virtual std::string getType() { return "NXPAV2"; };

        virtual void serialize(boost::property_tree::ptree& parentNode);
        virtual void unSerialize(boost::property_tree::ptree& node);
        virtual std::string getDefaultXmlNodeName() const { return "NXPAV2KeyDiversification"; };

        const std::vector<unsigned char>& getDivInput() const { return d_divInput; }

        void setDivInput(std::vector<unsigned char> divInput) { d_divInput = divInput; }

        const std::vector<unsigned char>& getSystemIdentifier() const { return d_systemIdentifier; }

        void setSystemIdentifier(std::vector<unsigned char> systemIdentifier) { d_systemIdentifier = systemIdentifier; }

    private:
        std::vector<unsigned char> d_divInput;

        std::vector<unsigned char> d_systemIdentifier;
    };
}

#endif