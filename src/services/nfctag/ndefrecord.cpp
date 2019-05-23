/**
 * \file ndefrecord.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief NDEF Record.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/services/nfctag/ndefrecord.hpp>
#include <logicalaccess/bufferhelper.hpp>

namespace logicalaccess
{
size_t NdefRecord::getEncodedSize() const
{
    size_t size = 0x02; // tnf + typeLength

    if (m_payload.size() > 0xFF)
        size += 0x04;
    else
        size += 0x01;

    if (m_id.size())
        size += 0x01;

    return size + m_type.size() + m_payload.size() + m_id.size();
}

void NdefRecord::init(TNF tnf, ByteVector type, ByteVector id, ByteVector payload)
{
    m_tnf = tnf;
    m_type = type;
    m_payload = payload;
    m_id = id;
}


ByteVector NdefRecord::encode(bool firstRecord, bool lastRecord)
{
    ByteVector data;

    data.push_back(getTnfByte(firstRecord, lastRecord));
    data.push_back(static_cast<unsigned char>(m_type.size()));

    if (m_payload.size() <= 0xFF) // short record
        data.push_back(static_cast<unsigned char>(m_payload.size()));
    else // long record
    {
        data.push_back(static_cast<uint8_t>((m_payload.size() & 0xff000000) >> 24));
        data.push_back(static_cast<uint8_t>((m_payload.size() & 0xff0000) >> 16));
        data.push_back((m_payload.size() & 0xff00) >> 8);
        data.push_back(m_payload.size() & 0xff);
    }

    if (m_id.size())
        data.push_back(static_cast<unsigned char>(m_id.size()));

    data.insert(data.end(), m_type.begin(), m_type.end());
    data.insert(data.end(), m_payload.begin(), m_payload.end());

    if (m_id.size())
        data.insert(data.end(), m_id.begin(), m_id.end());
    return data;
}

unsigned char NdefRecord::getTnfByte(bool firstRecord, bool lastRecord) const
{
    unsigned char value = static_cast<unsigned char>(m_tnf);

    if (firstRecord)
        value |= 0x80;

    if (lastRecord)
        value |= 0x40;

    if (m_payload.size() <= 0xFF)
        value |= 0x10;

    if (m_id.size())
        value |= 0x08;

    return value;
}

void NdefRecord::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("Tnf", (unsigned int)(getTnf()));
    node.put("Type", BufferHelper::getHex(getType()));
    node.put("Id", BufferHelper::getHex(getId()));
    node.put("Payload", BufferHelper::getHex(getPayload()));

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void NdefRecord::unSerialize(boost::property_tree::ptree &node)
{
    setTnf((TNF)(node.get_child("Tnf").get_value<unsigned int>()));
    setType(BufferHelper::fromHexString(node.get_child("Type").get_value<std::string>()));
    setId(BufferHelper::fromHexString(node.get_child("Id").get_value<std::string>()));
    setPayload(
        BufferHelper::fromHexString(node.get_child("Payload").get_value<std::string>()));
}
}
