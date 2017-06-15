/**
 * \file numberdatafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Number data field.
 */

#ifndef LOGICALACCESS_NUMBERDATAFIELD_HPP
#define LOGICALACCESS_NUMBERDATAFIELD_HPP

#include "logicalaccess/services/accesscontrol/formats/customformat/valuedatafield.hpp"

namespace logicalaccess
{
    /**
     * \brief A number data field.
     */
    class LIBLOGICALACCESS_API NumberDataField : public ValueDataField
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
         * \brief Set the field value.
         * \param value The field value.
         */
        void setValue(long long value);

        /**
         * \brief Get the field value.
         * \return The field value.
         */
        long long getValue() const;

        /**
         * \brief Get linear data.
         * \return data The linear datas in a bitsetstream
         */
        virtual std::vector<uint8_t> getLinearData() const;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         */
        virtual void setLinearData(const std::vector<uint8_t>& data);

        /**
         * \brief Check the current field skeleton with another field.
         * \param field The field to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(std::shared_ptr<DataField> field) const;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
        virtual void serialize(boost::property_tree::ptree& parentNode);

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
        virtual void unSerialize(boost::property_tree::ptree& node);

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
        virtual std::string getDefaultXmlNodeName() const;

    protected:

        long long d_value;
    };
}

#endif /* LOGICALACCESS_NUMBERDATAFIELD_HPP */