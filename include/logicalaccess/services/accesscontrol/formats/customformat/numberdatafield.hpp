/**
 * \file numberdatafield.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Number data field.
 */

#ifndef LOGICALACCESS_NUMBERDATAFIELD_HPP
#define LOGICALACCESS_NUMBERDATAFIELD_HPP

#include <logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp>
#include <logicalaccess/lla_core_api.hpp>

namespace logicalaccess
{
/**
 * \brief A number data field.
 */
class LLA_CORE_API NumberDataField : public ValueDataField
{
  public:
    /**
     * \brief Constructor.
     */
    NumberDataField();

    /**
     * \brief Destructor.
     */
    virtual ~NumberDataField();

    /**
     * \brief Get the field type.
     * \return The field type.
     */
    DataFieldType getDFType() const override
    {
        return DFT_NUMBER;
    }

    /**
     * \brief Set the field value.
     * \param value The field value.
     */
    void setValue(unsigned long long value);

    /**
     * \brief Get the field value.
     * \return The field value.
     */
    unsigned long long getValue() const;

    /**
     * \brief Get linear data.
     * \return data The linear datas in a bitsetstream
     */
    BitsetStream getLinearData(const BitsetStream &data) const override;

    /**
     * \brief Set linear data.
     * \param data Where to get data
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
    unsigned long long d_value;
};
}

#endif /* LOGICALACCESS_NUMBERDATAFIELD_HPP */