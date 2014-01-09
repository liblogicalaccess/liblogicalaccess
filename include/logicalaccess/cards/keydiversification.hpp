#include "logicalaccess/key.hpp"
#include "boost/shared_ptr.hpp"
#include <vector>

#ifndef KEYDIVERSIFICATION_HPP__
#define KEYDIVERSIFICATION_HPP__

namespace logicalaccess
{
	class Key;

	class LIBLOGICALACCESS_API KeyDiversification : public XmlSerializable
	{
	public:
		virtual void initDiversification(std::vector<unsigned char> d_identifier, int AID, boost::shared_ptr<Key> key, std::vector<unsigned char>& diversify) = 0;
		virtual std::vector<unsigned char> getDiversifiedKey(boost::shared_ptr<Key> key, std::vector<unsigned char> diversify) = 0;
		virtual std::string getType() = 0;

		static boost::shared_ptr<KeyDiversification> getKeyDiversificationFromType(std::string kdiv);
	};
}

#endif
