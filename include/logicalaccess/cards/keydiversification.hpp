#include "logicalaccess/key.hpp"
#include "boost/shared_ptr.hpp"
#include <vector>

#ifndef KEYDIVERSIFICATION_HPP__
#define KEYDIVERSIFICATION_HPP__

namespace logicalaccess
{
	class Key;

	class LIBLOGICALACCESS_API KeyDiversification
	{
	public:
		virtual bool initDiversification(unsigned char *diversify, std::vector<unsigned char> d_identifier, unsigned char *AID) = 0;
		virtual std::vector<unsigned char> getKeyDiversificated(boost::shared_ptr<Key> key, unsigned char* diversify) = 0;
	};
}

#endif