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
		virtual bool initDiversification(std::vector<unsigned char>& diversify, std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key);
		virtual std::vector<unsigned char> getKeyDiversificated(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify);
		NXPKeyDiversification() : d_systemidentifier("") {};
		NXPKeyDiversification(std::string systemidentifier) : d_systemidentifier(systemidentifier) {};
		~NXPKeyDiversification() {};

		std::string d_systemidentifier;
	};
}

#endif

