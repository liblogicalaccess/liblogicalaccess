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
		virtual bool initDiversification(std::vector<unsigned char>& diversify, std::vector<unsigned char> identifier, int AID, boost::shared_ptr<Key> key);
		virtual std::vector<unsigned char> getKeyDiversificated(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify);

		SagemKeyDiversification() {};
		~SagemKeyDiversification() {};
	};
}

#endif

