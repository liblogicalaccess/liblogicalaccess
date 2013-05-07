/**
 * \file TripleDESKey.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Triple DES Key.
 */

#include "logicalaccess/Cards/TripleDESKey.h"

namespace LOGICALACCESS
{
	TripleDESKey::TripleDESKey()
		: Key()
	{
		clear();
	}

	TripleDESKey::TripleDESKey(const std::string& str)
		: Key()
	{
		fromString(str);
	}

	TripleDESKey::TripleDESKey(const void* buf, size_t buflen)
		: Key()
	{
		clear();

		if (buf != NULL)
		{
			if (buflen >= TRIPLEDES_KEY_SIZE)
			{
				memcpy(d_key, buf, TRIPLEDES_KEY_SIZE);
				d_isEmpty = false;
			}
		}
	}

	void TripleDESKey::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void TripleDESKey::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);		
	}

	std::string TripleDESKey::getDefaultXmlNodeName() const
	{
		return "TripleDESKey";
	}
}