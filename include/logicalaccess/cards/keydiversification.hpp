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
		virtual bool initDiversification(std::vector<unsigned char>& diversify, std::vector<unsigned char> d_identifier, int AID, boost::shared_ptr<Key> key) = 0;
		virtual std::vector<unsigned char> getKeyDiversificated(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify) = 0;
	};
}

#endif