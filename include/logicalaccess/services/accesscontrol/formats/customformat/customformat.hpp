/**
 * \file customformat.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Custom format.
 */

#ifndef LOGICALACCESS_CUSTOMFORMAT_HPP
#define LOGICALACCESS_CUSTOMFORMAT_HPP

#include "logicalaccess/services/accesscontrol/formats/format.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp"

#include <list>

namespace logicalaccess
{
    /**
     * \brief A custom format.
     */
    class LIBLOGICALACCESS_API CustomFormat : public Format
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
        virtual unsigned int getDataLength() const;

        /**
         * \brief Set the format name.
         * \param name The format name.
         */
        virtual void setName(const string& name);

        /**
         * \brief Get the format name.
         * \return The format name.
         */
        virtual string getName() const;

        /**
         * \brief Get linear data.
         * \return The linear data.
         */
        virtual std::vector<uint8_t> getLinearData() const;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         */
        virtual void setLinearData(const std::vector<uint8_t>& data);

        /**
         * \brief Check the current format skeleton with another format.
         * \param format The format to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(std::shared_ptr<Format> format) const;

        /**
         * \brief Get the format type.
         * \return The format type.
         */
        virtual FormatType getType() const;

        /**
         * \brief Get skeleton linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
        virtual size_t getSkeletonLinearData(std::vector<uint8_t>& data) const;

        /**
         * \brief Set skeleton linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length in byte of data
         */
        virtual void setSkeletonLinearData(const std::vector<uint8_t>& data);

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

        /**
         * \brief Get the associated field for a specific position.
         * \param position The position.
         * \return The associated field at this position.
         */
        std::shared_ptr<DataField> getFieldForPosition(unsigned int position);

    protected:

        /**
         * \brief Reorder the fields list.
         */
        void reorderFields();

        /**
         * \brief The custom format name.
         */
        std::string d_name;
    };
}

#endif /* LOGICALACCESS_CUSTOMFORMAT_HPP */