/**
 * \file hmac1key.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief HMAC1 Key.
 */

#include "logicalaccess/cards/hmac1key.hpp"

namespace logicalaccess
{
	HMAC1Key::HMAC1Key()
		: Key()
	{
		memset(d_key, 0x00, sizeof(d_key));
		clear();
	}

	HMAC1Key::HMAC1Key(const std::string& str)
		: Key()
	{
		memset(d_key, 0x00, sizeof(d_key));
		fromString(str);
	}

	HMAC1Key::HMAC1Key(const void* buf, size_t buflen)
		: Key()
	{
		memset(d_key, 0x00, sizeof(d_key));
		clear();

		if (buf != NULL)
		{
			if (buflen >= HMAC1_KEY_SIZE)
			{
				memcpy(d_key, buf, HMAC1_KEY_SIZE);
				d_isEmpty = false;
			}
		}
	}

	void HMAC1Key::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void HMAC1Key::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);
	}

	std::string HMAC1Key::getDefaultXmlNodeName() const
	{
		return "HMAC1Key";
	}
}