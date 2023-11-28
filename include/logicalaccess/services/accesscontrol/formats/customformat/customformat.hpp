/**
 * \file customformat.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Custom format.
 */

#ifndef LOGICALACCESS_CUSTOMFORMAT_HPP
#define LOGICALACCESS_CUSTOMFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp>

#include <list>

namespace logicalaccess
{
/**
 * \brief A custom format.
 */
class LLA_CORE_API CustomFormat : public Format
{
  public:
    /**
     * \brief Constructor.
     */
    CustomFormat();

    /**
     * \brief Destructor.
     */
    virtual ~CustomFormat();

    /**
     * \brief Get data length in bits.
     * \return The data length in bits.
     */
    unsigned int getDataLength() const override;

    /**
     * \brief Set the format name.
     * \param name The format name.
     */
    virtual void setName(const std::string &name);

    /**
     * \brief Get the format name.
     * \return The format name.
     */
    std::string getName() const override;

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     */
    ByteVector getLinearData() const override;

    /**
     * \brief Set linear data.
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     */
    void setLinearData(const ByteVector &data) override;

    /**
     * \brief Check the current format skeleton with another format.
     * \param format The format to check.
     * \return True on success, false otherwise.
     */
    bool checkSkeleton(std::shared_ptr<Format> format) const override;

    /**
     * \brief Get the format type.
     * \return The format type.
     */
    FormatType getType() const override;

    /**
     * \brief Get skeleton linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     */
    size_t getSkeletonLinearData(ByteVector &data) const override;

    /**
     * \brief Set skeleton linear data.
     * \param data Where to get data
     * \param dataLengthBytes Length in byte of data
     */
    void setSkeletonLinearData(const ByteVector &data) override;

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

    /**
     * \brief Get the associated field for a specific position.
     * \param position The position.
     * \return The associated field at this position.
     */
    std::shared_ptr<DataField> getFieldForPosition(unsigned int position) const;

  protected:
    /**
     * \brief The custom format name.
     */
    std::string d_name;
};
}

#endif /* LOGICALACCESS_CUSTOMFORMAT_HPP */