/**
 * \file xmlserializable.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Xml Serializable.
 */

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include "logicalaccess/logs.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/xmlserializable.hpp"
#include <boost/property_tree/xml_parser.hpp>

namespace logicalaccess
{
unsigned long long XmlSerializable::atoull(const std::string &str)
{
    LOG(LogLevel::INFOS) << " ** ATOULL started for decimal string " << str << " len "
                         << str.size();
    unsigned long long n = 0;

    if (!str.empty())
    {
        const char *s = str.c_str();
        size_t strLen = str.size();

        for (size_t counter = 0; (counter < strLen) && ('0' <= *s) && (*s <= '9');
             ++counter)
        {
            n = n * 10 + *s++ - '0';
        }
    }

    LOG(LogLevel::INFOS) << " ** ATOULL returns " << n;

    return n;
}

ByteVector XmlSerializable::formatHexString(std::string hexstr)
{
    size_t buflen = hexstr.size() / 2;
    if (hexstr.size() % 2 == 0 && hexstr.size() > 2)
    {
        for (unsigned int i = 2; i <= hexstr.size() - 2; i += 2)
        {
            hexstr.insert(i, " ");
            i++;
        }
    }

    ByteVector buf;
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

void XmlSerializable::serializeToFile(const std::string &filename)
{
    std::ofstream ofs(filename.c_str(), std::ios_base::binary | std::ios_base::trunc);
    if (ofs.is_open())
    {
        std::string xmlstring = serialize();
        ofs.write(xmlstring.c_str(), xmlstring.length());
        ofs.close();
        if (ofs.bad())
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Writing serialize failed.");
    }
    else
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unable to open the file");
}

void XmlSerializable::unSerializeFromFile(const std::string &filename)
{
    std::ifstream ifs(filename.c_str(), std::ios_base::binary);
    if (!ifs.is_open())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unable to open the file");
    unSerialize(ifs, "");
}

std::string XmlSerializable::removeXmlDeclaration(const std::string &xmlstring)
{
    std::istringstream iss(xmlstring);
    std::string xmldeclaration, content;
    getline(iss, xmldeclaration);
    getline(iss, content);

    return content;
}

std::string XmlSerializable::serialize()
{
    return removeXmlDeclaration(serializeWithHeaders());
}

std::string XmlSerializable::serializeWithHeaders()
{
    std::ostringstream oss;
    boost::property_tree::ptree pt;

    serialize(pt);

    write_xml(oss, pt);
    return oss.str();
}

void XmlSerializable::unSerialize(const std::string &xmlstring,
                                  const std::string &rootNode)
{
    std::istringstream iss(xmlstring);
    unSerialize(iss, rootNode);
}

void XmlSerializable::unSerialize(std::istream &is, const std::string &rootNode)
{
    boost::property_tree::ptree pt;
    read_xml(is, pt);

    unSerialize(pt, rootNode);
}

void XmlSerializable::unSerialize(boost::property_tree::ptree &node,
                                  const std::string &rootNode)
{
    boost::property_tree::ptree nodep =
        node.get_child(rootNode + getDefaultXmlNodeName());
    unSerialize(nodep);
}
}