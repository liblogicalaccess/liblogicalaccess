#include "logicalaccess/cards/keydiversification.hpp"
#include "logicalaccess/key.hpp"
#include <vector>

#ifndef NXPKEYDIVERSIFICATION_HPP__
#define NXPKEYDIVERSIFICATION_HPP__

namespace logicalaccess
{
	class LIBLOGICALACCESS_API NXPKeyDiversification : public KeyDiversification
	{
	public:
		virtual bool initDiversification(unsigned char *diversify, std::vector<unsigned char> identifier, unsigned char *AID);
		virtual std::vector<unsigned char> getKeyDiversificated(boost::shared_ptr<Key> key, unsigned char* diversify);
		NXPKeyDiversification() {};
		~NXPKeyDiversification() {};

	private:
		std::vector<unsigned char> NXPKeyDiversification::XORVector(std::vector<unsigned char> str1, std::vector<unsigned char> str2);
	};
}

#endif

