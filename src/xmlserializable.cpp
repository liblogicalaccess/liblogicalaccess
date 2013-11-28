/**
 * \file xmlserializable.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Xml Serializable.
 */

#include "logicalaccess/myexception.hpp"
#include "logicalaccess/xmlserializable.hpp"
#include "logicalaccess/logs.hpp"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <boost/algorithm/string.hpp>

namespace logicalaccess
{
	unsigned long long XmlSerializable::atoull(const std::string& str)
	{
		INFO_(" ** ATOULL started for decimal string {%s} len {%d}", str.c_str(), str.size());
		unsigned long long n = 0;
		
		if (!str.empty())
		{
			const char *s = str.c_str();
			size_t strLen = str.size();

			for (size_t counter = 0; (counter < strLen) && ('0' <= *s) && (*s <= '9') ; ++counter)
			{
				n = n*10 + *s++ - '0';
			}
		}

		INFO_(" ** ATOULL returns {%llu}", n);

		return n;
	}

	std::vector<unsigned char> XmlSerializable::formatHexString(std::string hexstr)
	{
		size_t buflen = hexstr.size() / 2;
		if (hexstr.size() % 2 == 0 && hexstr.size() > 2)
		{
			for (unsigned int i = 2; i <= hexstr.size()-2; i+=2)
			{
				hexstr.insert(i, " ");
				i++;
			}
		}

		std::vector<unsigned char> buf;
		std::istringstream iss(hexstr);
		for (unsigned int i = 0; i < buflen; ++i)
		{
			unsigned int tmp;

			if (!iss.good())
			{
				buf.clear();
				break;
			}

			iss >> std::hex >> tmp;
			buf.push_back(static_cast<unsigned char>(tmp));
		}
		
		return buf;
	}

	void XmlSerializable::serializeToFile(const std::string& filename)
	{
		std::ofstream ofs(filename.c_str(), std::ios_base::binary | std::ios_base::trunc);
		if (ofs.is_open())
		{
			std::string xmlstring = serialize();
			ofs.write(xmlstring.c_str(), xmlstring.length());
			ofs.close();
			if (ofs.bad())
				THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Writing serialize failed.");
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unable to open the file");
	}

	void XmlSerializable::unSerializeFromFile(const std::string& filename)
	{
		std::ifstream ifs(filename.c_str(), std::ios_base::binary);
		if (!ifs.is_open())
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unable to open the file");
		unSerialize(ifs, "");
	}

	std::string XmlSerializable::removeXmlDeclaration(const std::string& xmlstring)
	{
		std::istringstream iss(xmlstring);
		std::string xmldeclaration, content;
		std::getline(iss, xmldeclaration);
		std::getline(iss, content);

		return content;
	}

	std::string XmlSerializable::serialize()
	{
		return XmlSerializable::removeXmlDeclaration(serializeWithHeaders());
	}

	std::string XmlSerializable::serializeWithHeaders()
	{
		std::ostringstream oss;
		boost::property_tree::ptree pt;

		serialize(pt);

		boost::property_tree::xml_parser::write_xml(oss, pt);
		return oss.str();
	}

	void XmlSerializable::unSerialize(const std::string& xmlstring, const std::string& rootNode)
	{
		std::istringstream iss(xmlstring);
		unSerialize(iss, rootNode);
	}

	void XmlSerializable::unSerialize(std::istream& is, const std::string& rootNode)
	{
		boost::property_tree::ptree pt;
		boost::property_tree::xml_parser::read_xml(is, pt);
			
		unSerialize(pt, rootNode);
	}

	void XmlSerializable::unSerialize(boost::property_tree::ptree& node, const std::string& rootNode)
	{
		boost::property_tree::ptree nodep = node.get_child(rootNode + getDefaultXmlNodeName());
		if (nodep.empty())
			THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, std::string("unSerialize ") + rootNode + getDefaultXmlNodeName() + std::string(" failed (node empty)."));
		unSerialize(nodep);
	}
}

