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

        NXPAV2KeyDiversification() : d_revertAID(false), d_forceK2Use(false) {};
        NXPAV2KeyDiversification(const std::vector<unsigned char>& divInput) : d_revertAID(false), d_divInput(divInput) {};
        virtual ~NXPAV2KeyDiversification() {};

        virtual std::string getType() { return "NXPAV2"; };

        virtual void serialize(boost::property_tree::ptree& parentNode);
        virtual void unSerialize(boost::property_tree::ptree& node);
        virtual std::string getDefaultXmlNodeName() const { return "NXPAV2KeyDiversification"; };

		bool getRevertAID() const { return d_revertAID; }

		void setRevertAID(bool revertAID) { d_revertAID = revertAID; }

        bool getForceK2Use() const { return d_forceK2Use; }

        void setForceK2Use(bool forceK2Use) { d_forceK2Use = forceK2Use; }

        const std::vector<unsigned char>& getDivInput() const { return d_divInput; }

        void setDivInput(std::vector<unsigned char> divInput) { d_divInput = divInput; }

        const std::vector<unsigned char>& getSystemIdentifier() const { return d_systemIdentifier; }

        void setSystemIdentifier(std::vector<unsigned char> systemIdentifier) { d_systemIdentifier = systemIdentifier; }

    private:
		bool d_revertAID;

        std::vector<unsigned char> d_divInput;

        std::vector<unsigned char> d_systemIdentifier;

        bool d_forceK2Use;
    };
}

#endif