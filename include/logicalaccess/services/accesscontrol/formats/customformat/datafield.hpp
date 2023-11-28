/**
 * \file datafield.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Data field.
 */

#ifndef LOGICALACCESS_DATAFIELD_HPP
#define LOGICALACCESS_DATAFIELD_HPP

#include <logicalaccess/xmlserializable.hpp>

#include <memory>
#include <boost/utility.hpp>

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/services/accesscontrol/formats/BitsetStream.hpp>

#define UNKNOWN_FIELD_POSITION 0xffffffff

namespace logicalaccess
{
typedef enum {
    DFT_VALUE    = 0x00, /* ValueDataField */
    DFT_CHECKSUM = 0x01, /* ChecksumDataField */
    DFT_PARITY   = 0x02, /* ParityDataField */
    DFT_BINARY   = 0x03, /* BinaryDataField */
    DFT_NUMBER   = 0x04, /* NumberDataField */
    DFT_STRING   = 0x05, /* StringDataField */
	DFT_TLV      = 0x06  /* TLVDataField */
} DataFieldType;
/**
 * \brief A data field base class.
 */
class LLA_CORE_API DataField : public XmlSerializable,
                                       public std::enable_shared_from_this<DataField>
{
  public:
    /**
     * \brief Constructor.
     */
    DataField();

    /**
     * \brief Destructor.
     */
    virtual ~DataField();

    /**
     * \brief Get data length in bits.
     * \return The data length in bits.
     */
    virtual unsigned int getDataLength() const;

    /**
     * \brief Set the field position in bits.
     * \param position The field position in bits.
     */
    virtual void setPosition(unsigned int position);

    /**
     * \brief Get the field position in bits.
     * \return The field position in bits.
     */
    unsigned int getPosition() const;

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    virtual DataFieldType getDFType() const = 0;

    /**
     * \brief Set the field name.
     * \param name The field name.
     */
    void setName(const std::string &name);

    /**
     * \brief Get the field name.
     * \return The field name.
     */
    std::string getName() const;

    /**
     * \brief Get linear data.
     * \return data The linear datas in a bitsetstream
     */
    virtual BitsetStream getLinearData(const BitsetStream &data) const = 0;

    /**
     * \brief Set linear data.
     * \param data Where to get data
     */
    virtual void setLinearData(const ByteVector &data) = 0;

    /**
     * \brief Check the current field skeleton with another field.
     * \param field The field to check.
     * \return True on success, false otherwise.
     */
    virtual bool checkSkeleton(std::shared_ptr<DataField> field) const = 0;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

  protected:
    /**
     * \brief The field name.
     */
    std::string d_name;

    /**
     * \brief The field length in bits.
     */
    unsigned int d_length;

    /**
     * \brief The field position in bits.
     */
    unsigned int d_position;
};
}

#endif /* LOGICALACCESS_DATAFIELD_HPP */
