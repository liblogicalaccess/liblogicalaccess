/**
 * \file paritydatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Parity data field. The parity is calculate using other fields.
 */

#ifndef LOGICALACCESS_PARITYDATAFIELD_HPP
#define LOGICALACCESS_PARITYDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp>
#include <logicalaccess/services/accesscontrol/encodings/datatype.hpp>

namespace logicalaccess
{
/**
 * \brief A parity data field.
 */
class LIBLOGICALACCESS_API ParityDataField : public DataField
{
  public:
    /**
     * \brief Constructor.
     */
    ParityDataField();

    /**
     * \brief Destructor.
     */
    virtual ~ParityDataField();

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    DataFieldType getDFType() const override
    {
        return DFT_PARITY;
    }

    /**
     * \brief Set the field position in bits.
     * \param position The field position in bits.
     */
    void setPosition(unsigned int position) override;

    /**
     * \brief Set the parity type.
     * \param type The parity type.
     */
    void setParityType(ParityType type);

    /**
     * \brief Get the parity type.
     * \return The parity type.
     */
    ParityType getParityType() const;

    /**
     * \brief Set the bits to use positions to calculate parity.
     * \param positions The bits positions.
     */
    void setBitsUsePositions(std::vector<unsigned int> positions);

    /**
     * \brief Get the bits to use positions to calculate parity.
     * \return The bits positions.
     */
    std::vector<unsigned int> getBitsUsePositions() const;

    /**
     * \brief Check the field dependecy with another one.
     * \param field The field to check with.
     * \return True if the field is dependent, false otherwise.
     */
    bool checkFieldDependecy(std::shared_ptr<DataField> field);

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    BitsetStream getLinearData(const BitsetStream &data) const override;

    /**
     * \brief Set linear data.
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     * \param pos The first position bit. Will contain the position bit after the field.
     */
    void setLinearData(const ByteVector &data) override;

    /**
     * \brief Check the current field skeleton with another field.
     * \param field The field to check.
     * \return True on success, false otherwise.
     */
    bool checkSkeleton(std::shared_ptr<DataField> field) const override;

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

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

  protected:
    ParityType d_parityType;

    std::vector<unsigned int> d_bitsUsePositions;
};
}

#endif /* LOGICALACCESS_PARITYDATAFIELD_HPP */