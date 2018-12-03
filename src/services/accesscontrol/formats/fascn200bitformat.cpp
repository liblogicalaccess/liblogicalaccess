/**
 * \file fascn200bitformat.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FASC-N 200 Bit format.
 */

#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>
#include <logicalaccess/services/accesscontrol/encodings/bcdnibbledatatype.hpp>
#include <logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>

namespace logicalaccess
{
const unsigned char FASCN200BitFormat::FASCN_SS = 0x0b;
const unsigned char FASCN200BitFormat::FASCN_FS = 0x0d;
const unsigned char FASCN200BitFormat::FASCN_ES = 0x0f;

const unsigned char FASCN200BitFormat::FASCN_SS_WITH_PARITY = 0x1A;
const unsigned char FASCN200BitFormat::FASCN_FS_WITH_PARITY = 0x16;
const unsigned char FASCN200BitFormat::FASCN_ES_WITH_PARITY = 0x1F;

FASCN200BitFormat::FASCN200BitFormat()
    : StaticFormat()
{
    memset(&d_formatLinear, 0x00, sizeof(d_formatLinear));
    d_dataType.reset(new BCDNibbleDataType());
    d_dataRepresentation.reset(new NoDataRepresentation());

    d_dataType->setBitDataRepresentationType(ET_LITTLEENDIAN);
    d_dataType->setRightParityType(PT_ODD);

    std::shared_ptr<NumberDataField> field(new NumberDataField());
    field->setName("Uid");
    field->setIsIdentifier(true);
    field->setDataLength(24);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("AgencyCode");
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("SystemCode");
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("PersonIdentifier");
    field->setDataLength(40);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("OrganizationalCategory");
    field->setDataLength(4);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("OrganizationalIdentifier");
    field->setDataLength(16);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);

    field = std::make_shared<NumberDataField>();
    field->setName("POACategory");
    field->setDataLength(4);
    field->setDataRepresentation(d_dataRepresentation);
    field->setDataType(d_dataType);
    d_fieldList.push_back(field);
}

FASCN200BitFormat::~FASCN200BitFormat()
{
}

unsigned short FASCN200BitFormat::getAgencyCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("AgencyCode"));
    return static_cast<unsigned short>(field->getValue());
}

void FASCN200BitFormat::setAgencyCode(unsigned short agencyCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("AgencyCode"));
    field->setValue(agencyCode);
    d_formatLinear.d_agencyCode = agencyCode;
}

unsigned short FASCN200BitFormat::getSystemCode() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("SystemCode"));
    return static_cast<unsigned short>(field->getValue());
}

void FASCN200BitFormat::setSystemCode(unsigned short systemCode)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("SystemCode"));
    field->setValue(systemCode);
    d_formatLinear.d_systemCode = systemCode;
}

unsigned char FASCN200BitFormat::getSerieCode() const
{
    return d_formatLinear.d_serieCode;
}

void FASCN200BitFormat::setSerieCode(unsigned char serieCode)
{
    d_formatLinear.d_serieCode = serieCode;
}

unsigned char FASCN200BitFormat::getCredentialCode() const
{
    return d_formatLinear.d_credentialCode;
}

void FASCN200BitFormat::setCredentialCode(unsigned char credentialCode)
{
    d_formatLinear.d_credentialCode = credentialCode;
}

unsigned long long FASCN200BitFormat::getPersonIdentifier() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("PersonIdentifier"));
    return static_cast<unsigned long long>(field->getValue());
}

void FASCN200BitFormat::setPersonIdentifier(unsigned long long personIdentifier)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("PersonIdentifier"));
    field->setValue(personIdentifier);
    d_formatLinear.d_personIdentifier = personIdentifier;
}

FASCNOrganizationalCategory FASCN200BitFormat::getOrganizationalCategory() const
{
    std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(
        getFieldFromName("OrganizationalCategory"));
    return static_cast<FASCNOrganizationalCategory>(field->getValue());
}

void FASCN200BitFormat::setOrganizationalCategory(FASCNOrganizationalCategory orgCategory)
{
    std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(
        getFieldFromName("OrganizationalCategory"));
    field->setValue(orgCategory);
    d_formatLinear.d_orgCategory = orgCategory;
}

unsigned short FASCN200BitFormat::getOrganizationalIdentifier() const
{
    std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(
        getFieldFromName("OrganizationalIdentifier"));
    return static_cast<unsigned short>(field->getValue());
}

void FASCN200BitFormat::setOrganizationalIdentifier(unsigned short orgIdentifier)
{
    std::shared_ptr<NumberDataField> field = std::dynamic_pointer_cast<NumberDataField>(
        getFieldFromName("OrganizationalIdentifier"));
    field->setValue(orgIdentifier);
    d_formatLinear.d_orgIdentifier = orgIdentifier;
}

FASCNPOAssociationCategory FASCN200BitFormat::getPOACategory() const
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("POACategory"));
    return static_cast<FASCNPOAssociationCategory>(field->getValue());
}

void FASCN200BitFormat::setPOACategory(FASCNPOAssociationCategory poaCategory)
{
    std::shared_ptr<NumberDataField> field =
        std::dynamic_pointer_cast<NumberDataField>(getFieldFromName("POACategory"));
    field->setValue(poaCategory);
    d_formatLinear.d_poaCategory = poaCategory;
}

unsigned int FASCN200BitFormat::getDataLength() const
{
    return 200;
}

std::string FASCN200BitFormat::getName() const
{
    return std::string("FASC-N 200 Bit");
}

FormatType FASCN200BitFormat::getType() const
{
    return FT_FASCN200BIT;
}

void FASCN200BitFormat::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getType());
    node.put("AgencyCode", getAgencyCode());
    node.put("SystemCode", getSystemCode());
    node.put("SerieCode", d_formatLinear.d_serieCode);
    node.put("CredentialCode", d_formatLinear.d_credentialCode);
    node.put("PersonIdentifier", getPersonIdentifier());
    node.put("OrganizationalCategory", getOrganizationalCategory());
    node.put("OrganizationalIdentifier", getOrganizationalIdentifier());
    node.put("POACategory", getPOACategory());
    node.put("Uid", getUid());

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void FASCN200BitFormat::unSerialize(boost::property_tree::ptree &node)
{
    setAgencyCode(node.get_child("AgencyCode").get_value<short>());
    setSystemCode(node.get_child("SystemCode").get_value<short>());
    setSerieCode(node.get_child("SerieCode").get_value<unsigned char>());
    setCredentialCode(node.get_child("CredentialCode").get_value<unsigned char>());
    setPersonIdentifier(
        node.get_child("PersonIdentifier").get_value<unsigned long long>());
    setOrganizationalCategory(static_cast<FASCNOrganizationalCategory>(
        node.get_child("OrganizationalCategory").get_value<unsigned int>()));
    setOrganizationalIdentifier(
        node.get_child("OrganizationalIdentifier").get_value<short>());
    setPOACategory(static_cast<FASCNPOAssociationCategory>(
        node.get_child("POACategory").get_value<unsigned int>()));
    setUid(node.get_child("Uid").get_value<unsigned long long>());
}

std::string FASCN200BitFormat::getDefaultXmlNodeName() const
{
    return "FASCN200BitFormat";
}

ByteVector FASCN200BitFormat::getLinearData() const
{
    BitsetStream data;

    data.append(FASCN_SS_WITH_PARITY, 3, 5);
    convertField(data, getAgencyCode(), 16);
    data.append(FASCN_FS_WITH_PARITY, 3, 5);
    convertField(data, getSystemCode(), 16);
    data.append(FASCN_FS_WITH_PARITY, 3, 5);
    convertField(data, getUid(), 24);
    data.append(FASCN_FS_WITH_PARITY, 3, 5);
    convertField(data, getSerieCode(), 4);
    data.append(FASCN_FS_WITH_PARITY, 3, 5);
    convertField(data, getCredentialCode(), 4);
    data.append(FASCN_FS_WITH_PARITY, 3, 5);
    convertField(data, getPersonIdentifier(), 40);
    convertField(data, getOrganizationalCategory(), 4);
    convertField(data, getOrganizationalIdentifier(), 16);
    convertField(data, getPOACategory(), 4);
    data.append(FASCN_ES_WITH_PARITY, 3, 5);
    unsigned char lrc = calculateLRC(data);
    convertField(data, lrc, 4);

    return data.getData();
}

void FASCN200BitFormat::setLinearData(const ByteVector &data)
{
    unsigned int pos = 0;
    BitsetStream _data;
    _data.concat(data);

    if (_data.getByteSize() * 8 < getDataLength())
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Data length too small.");
    }

    unsigned char c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_SS)
    {
        char tmpmsg[64];
        sprintf(tmpmsg, "The FASC-N Start Sentinel doesn't match (%x).", c);
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, tmpmsg);
    }
    ++pos;
    setAgencyCode((unsigned short)revertField(_data, &pos, 16));
    pos += 4;
    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_FS)
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The FASC-N Field Separator doesn't match after the Agency Code.");
    }
    ++pos;

    setSystemCode((unsigned short)revertField(_data, &pos, 16));
    pos += 4;
    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_FS)
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The FASC-N Field Separator doesn't match after the System Code.");
    }
    ++pos;

    setUid(revertField(_data, &pos, 24));
    pos += 6;
    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_FS)
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The FASC-N Field Separator doesn't match after the Credential.");
    }
    ++pos;

    setSerieCode((unsigned char)revertField(_data, &pos, 4));
    ++pos;
    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_FS)
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The FASC-N Field Separator doesn't match after the Credential Series.");
    }
    ++pos;

    setCredentialCode((unsigned char)revertField(_data, &pos, 4));
    ++pos;
    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_FS)
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "The FASC-N Field Separator doesn't match after the Credential Issue.");
    }
    ++pos;

    setPersonIdentifier(revertField(_data, &pos, 40));
    pos += 10;

    setOrganizationalCategory((FASCNOrganizationalCategory)revertField(_data, &pos, 4));
    ++pos;
    setOrganizationalIdentifier(
        static_cast<unsigned short>(revertField(_data, &pos, 16)));
    pos += 4;
    setPOACategory((FASCNPOAssociationCategory)revertField(_data, &pos, 4));
    ++pos;

    c = (unsigned char)revertField(_data, &pos, 4);
    if (c != FASCN_ES)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "The End Sentinel doesn't match.");
    }
    ++pos;

    // We do not find the correct LRC for now...
    /*
    unsigned char lrc = calculateLRC(_data);
    c = (unsigned char)revertField(_data, &pos, 4);



    if (c != lrc)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The FASC-N LRC seems
    wrong.");
    }*/
}

unsigned char FASCN200BitFormat::calculateLRC(const BitsetStream &data) const
{
    unsigned char lrc = 0x00;
    BitsetStream tmp  = data;
    unsigned int pos  = 0;


    if (data.getByteSize() > 1)
    {
        while (pos < tmp.getBitSize())
        {
            unsigned char c = (unsigned char)revertField(tmp, &pos, 4);
            ++pos; // We skip the parity
            lrc ^= c;
        }
    }

    return lrc;
}

size_t FASCN200BitFormat::getFormatLinearData(ByteVector &data) const
{
    size_t retLength = sizeof(d_formatLinear);

    if (data.size() >= retLength)
    {
        // size_t pos = 0;
        // memcpy(&reinterpret_cast<unsigned char*>(data)[pos], &d_formatLinear,
        // sizeof(d_formatLinear));
        memcpy(&data[0], &d_formatLinear, sizeof(d_formatLinear));
    }

    return retLength;
}

void FASCN200BitFormat::setFormatLinearData(const ByteVector &data, size_t *indexByte)
{
    // memcpy(&d_formatLinear, &reinterpret_cast<const unsigned char*>(data)[*indexByte],
    // sizeof(d_formatLinear));
    memcpy(&d_formatLinear, &data[*indexByte], sizeof(d_formatLinear));
    (*indexByte) += sizeof(d_formatLinear);

    setAgencyCode(d_formatLinear.d_agencyCode);
    setOrganizationalCategory(d_formatLinear.d_orgCategory);
    setOrganizationalIdentifier(d_formatLinear.d_orgIdentifier);
    setPersonIdentifier(d_formatLinear.d_personIdentifier);
    setPOACategory(d_formatLinear.d_poaCategory);
    setSystemCode(d_formatLinear.d_systemCode);
}

bool FASCN200BitFormat::checkSkeleton(std::shared_ptr<Format> format) const
{
    bool ret = false;
    if (format)
    {
        std::shared_ptr<FASCN200BitFormat> pFormat =
            std::dynamic_pointer_cast<FASCN200BitFormat>(format);
        if (pFormat)
        {
            ret = ((d_formatLinear.d_agencyCode == 0 ||
                    d_formatLinear.d_agencyCode == pFormat->getAgencyCode()) &&
                   (d_formatLinear.d_systemCode == 0 ||
                    d_formatLinear.d_systemCode == pFormat->getSystemCode()));
        }
    }
    return ret;
}
}