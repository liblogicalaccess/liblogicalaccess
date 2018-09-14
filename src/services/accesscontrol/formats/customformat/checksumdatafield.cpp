/**
 * \file checksumdatafield.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Checksum Data field.
 */

#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/checksumdatafield.hpp>

namespace logicalaccess
{
ChecksumDataField::ChecksumDataField()
    : DataField()
{
}

ChecksumDataField::~ChecksumDataField()
{
}

void ChecksumDataField::setValue(const char & /*value*/)
{
    throw std::runtime_error("Unimplemented");
}

unsigned char ChecksumDataField::getValue() const
{
    throw std::runtime_error("Unimplemented");
}

void ChecksumDataField::setBitsUsePositions(ByteVector /*positions*/)
{
    throw std::runtime_error("Unimplemented");
}

ByteVector ChecksumDataField::getBitsUsePositions() const
{
    throw std::runtime_error("Unimplemented");
}

BitsetStream ChecksumDataField::getLinearData(const BitsetStream &data) const
{
    throw std::runtime_error("Unimplemented");
}

void ChecksumDataField::setLinearData(const ByteVector &data)
{
    throw std::runtime_error("Unimplemented");
}

void ChecksumDataField::serialize(boost::property_tree::ptree &parentNode)
{
    throw std::runtime_error("Unimplemented");
}

void ChecksumDataField::unSerialize(boost::property_tree::ptree &node)
{
    throw std::runtime_error("Unimplemented");
}

std::string ChecksumDataField::getDefaultXmlNodeName() const
{
    return "ChecksumDataField";
}
}