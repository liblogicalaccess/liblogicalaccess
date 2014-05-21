#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include <vector>

#ifndef NXPKEYDIVERSIFICATION_HPP__
#define NXPKEYDIVERSIFICATION_HPP__

#include <string>

namespace logicalaccess
{
	class LIBLOGICALACCESS_API NXPAV2KeyDiversification : public KeyDiversification
	{
	public:
		virtual void initDiversification(std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key, unsigned char keyno, std::vector<unsigned char>& diversify);
		virtual std::vector<unsigned char> getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify);

		NXPAV2KeyDiversification() {};
		NXPAV2KeyDiversification(const std::vector<unsigned char>& systemidentifier) : d_divInput(systemidentifier) {};
		~NXPAV2KeyDiversification() {};

		virtual std::string getType() { return "NXPAV2"; };

		virtual void serialize(boost::property_tree::ptree& parentNode);
		virtual void unSerialize(boost::property_tree::ptree& node);
		virtual std::string getDefaultXmlNodeName() const { return "NXPAV2KeyDiversification"; };

		const std::vector<unsigned char>& getDivInput() const { return d_divInput; }

		void setDivInput(std::vector<unsigned char> divInput) { d_divInput = divInput; }

	private:
		std::vector<unsigned char> d_divInput;
	};
}

#endif

