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
		virtual void initDiversification(std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key, std::vector<unsigned char>& diversify);
		virtual std::vector<unsigned char> getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify);

		NXPKeyDiversification() : d_systemidentifier("") {};
		NXPKeyDiversification(std::string systemidentifier) : d_systemidentifier(systemidentifier) {};
		~NXPKeyDiversification() {};

		virtual std::string getType() { return "NXP"; };

		std::string d_systemidentifier;

		virtual void serialize(boost::property_tree::ptree& parentNode);
		virtual void unSerialize(boost::property_tree::ptree& node);
		virtual std::string getDefaultXmlNodeName() const { return "NXPKeyDiversification"; };
	};
}

#endif

