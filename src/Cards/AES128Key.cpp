/**
 * \file AES128Key.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief AES128 Key.
 */

#include "logicalaccess/Cards/AES128Key.h"

namespace LOGICALACCESS
{
	AES128Key::AES128Key()
		: Key()
	{
		clear();
	}

	AES128Key::AES128Key(const std::string& str)
		: Key()
	{
		fromString(str);
	}

	AES128Key::AES128Key(const void* buf, size_t buflen)
		: Key()
	{
		clear();

		if (buf != NULL)
		{
			if (buflen >= AES128_KEY_SIZE)
			{
				memcpy(d_key, buf, AES128_KEY_SIZE);
				d_isEmpty = false;
			}
		}
	}

	void AES128Key::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;
		Key::serialize(node);
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void AES128Key::unSerialize(boost::property_tree::ptree& node)
	{
		Key::unSerialize(node);		
	}

	std::string AES128Key::getDefaultXmlNodeName() const
	{
		return "AES128Key";
	}
}
