/**
 * \file iso7816location.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location informations for ISO7816 card.
 */

#include "iso7816location.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <cstdlib>
#include <cstring>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
ISO7816Location::ISO7816Location()
{
    memset(dfname, 0x00, sizeof(dfname));
    dfnamelen  = 0;
    fileid     = 0;
    fileType   = IFT_NONE;
    dataObject = 0;
}

ISO7816Location::~ISO7816Location()
{
}

void ISO7816Location::serialize(boost::property_tree::ptree &parentNode)
{
    ByteVector buf(dfname, dfname + sizeof(dfnamelen));
    boost::property_tree::ptree node;

    node.put("DFName", BufferHelper::getHex(buf));
    node.put("FileID", fileid);
    node.put("FileType", fileType);
    node.put("DataObject", dataObject);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void ISO7816Location::unSerialize(boost::property_tree::ptree &node)
{
    std::string dfnamestr = node.get_child("DFName").get_value<std::string>();
    ByteVector buf        = BufferHelper::fromHexString(dfnamestr);
    if (buf.size() <= 16)
    {
        dfnamelen = static_cast<int>(buf.size());
        memcpy(dfname, &buf[0], buf.size());
        fileid   = node.get_child("FileID").get_value<unsigned short>();
        fileType = static_cast<ISO7816FileType>(
            node.get_child("FileType").get_value<unsigned int>());
        dataObject = node.get_child("DataObject").get_value<unsigned short>();
    }
}

std::string ISO7816Location::getDefaultXmlNodeName() const
{
    return "ISO7816Location";
}

bool ISO7816Location::operator==(const Location &location) const
{
    if (!Location::operator==(location))
        return false;

    const ISO7816Location *pxLocation = dynamic_cast<const ISO7816Location *>(&location);

    if (!pxLocation)
        return false;

    return (fileid == pxLocation->fileid && fileType == pxLocation->fileType &&
            dataObject == pxLocation->dataObject && dfnamelen == pxLocation->dfnamelen &&
            memcmp(dfname, pxLocation->dfname, sizeof(dfname)) == 0);
}
}