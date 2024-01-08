/**
 * \file paritydatafield.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Parity Data field.
 */

#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/paritydatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
ParityDataField::ParityDataField()
    : DataField()
{
    d_length     = 1;
    d_parityType = PT_EVEN;
}

ParityDataField::~ParityDataField()
{
}

void ParityDataField::setPosition(unsigned int position)
{
    ByteVector newPositions;
    for (std::vector<unsigned int>::iterator i = d_bitsUsePositions.begin();
         i != d_bitsUsePositions.end(); ++i)
    {
        if (d_position < (*i) && position > (*i))
        {
            newPositions.push_back((*i) - d_length);
        }
        else if (d_position > (*i) && position < (*i))
        {
            newPositions.push_back((*i) + d_length);
        }
        else
        {
            newPositions.push_back((*i));
        }
    }

    DataField::setPosition(position);
}

void ParityDataField::setParityType(ParityType type)
{
    d_parityType = type;
}

ParityType ParityDataField::getParityType() const
{
    return d_parityType;
}

void ParityDataField::setBitsUsePositions(std::vector<unsigned int> positions)
{
    d_bitsUsePositions = positions;
}

std::vector<unsigned int> ParityDataField::getBitsUsePositions() const
{
    return d_bitsUsePositions;
}

BitsetStream ParityDataField::getLinearData(const BitsetStream &data) const
{
    BitsetStream dataTmp;
    std::vector<unsigned int> positions(d_bitsUsePositions.size());
    int i = 0;
    for (std::vector<unsigned int>::const_iterator b = d_bitsUsePositions.begin();
         b != d_bitsUsePositions.end(); ++b)
    {
        positions[i++] = (*b);
    }
    unsigned char parity = Format::calculateParity(data, d_parityType, positions);
    dataTmp.append(parity, 7, 1);
    return dataTmp;
}

void ParityDataField::setLinearData(const ByteVector &data)
{
    BitsetStream _data;
    _data.concat(data);

    if (_data.getBitSize() < (d_length + getPosition()))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The data length is too short.");
    }

    std::vector<unsigned int> positions(d_bitsUsePositions.size());
    int i = 0;
    for (std::vector<unsigned int>::const_iterator b = d_bitsUsePositions.begin();
         b != d_bitsUsePositions.end(); ++b)
    {
        positions[i++] = (*b);
    }
    unsigned char parity = Format::calculateParity(_data, d_parityType, positions);
    unsigned char currentParity =
        (unsigned char)((data[getPosition() / 8] << (getPosition() % 8)) >> 7);

    if (parity != currentParity)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The parity " + getName() + " doesn't match.");
    }
}

bool ParityDataField::checkSkeleton(std::shared_ptr<DataField> field) const
{
    bool ret = false;
    if (field)
    {
        std::shared_ptr<ParityDataField> pField =
            std::dynamic_pointer_cast<ParityDataField>(field);
        if (pField)
        {
            std::vector<unsigned int> bitsPositions = pField->getBitsUsePositions();

            ret = (pField->getDataLength() == getDataLength() &&
                   pField->getPosition() == getPosition() &&
                   pField->getParityType() == getParityType() &&
                   bitsPositions.size() == d_bitsUsePositions.size());

            if (ret)
            {
                for (std::vector<unsigned int>::const_iterator
                         i  = d_bitsUsePositions.begin(),
                         fi = bitsPositions.begin();
                     ret && i != d_bitsUsePositions.end() && fi != bitsPositions.end();
                     ++i, ++fi)
                {
                    ret = (*i) == (*fi);
                }
            }
        }
    }

    return ret;
}

bool ParityDataField::checkFieldDependecy(std::shared_ptr<DataField> field)
{
    bool depend = false;

    for (std::vector<unsigned int>::const_iterator i = d_bitsUsePositions.begin();
         !depend && i != d_bitsUsePositions.end(); ++i)
    {
        depend = (((*i) >= field->getPosition()) &&
                  ((*i) < (field->getPosition() + field->getDataLength())));
    }

    return depend;
}

void ParityDataField::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    DataField::serialize(node);
    node.put("ParityType", d_parityType);
    boost::property_tree::ptree bnode;
    for (std::vector<unsigned int>::iterator i = d_bitsUsePositions.begin();
         i != d_bitsUsePositions.end(); ++i)
    {
        bnode.add("Bit", *i);
    }
    node.add_child("BitsPositions", bnode);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void ParityDataField::unSerialize(boost::property_tree::ptree &node)
{
    d_bitsUsePositions.clear();
    DataField::unSerialize(node);
    d_parityType =
        static_cast<ParityType>(node.get_child("ParityType").get_value<unsigned int>());
    BOOST_FOREACH (boost::property_tree::ptree::value_type const &v,
                   node.get_child("BitsPositions"))
    {
        unsigned int bit = v.second.get_value<unsigned int>();
        d_bitsUsePositions.push_back(bit);
    }
}

std::string ParityDataField::getDefaultXmlNodeName() const
{
    return "ParityDataField";
}
}