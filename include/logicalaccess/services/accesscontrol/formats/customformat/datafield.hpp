/**
 * \file datafield.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Data field.
 */

#ifndef LOGICALACCESS_DATAFIELD_HPP
#define LOGICALACCESS_DATAFIELD_HPP

#include "logicalaccess/xmlserializable.hpp"

#include <memory>
#include <boost/utility.hpp>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief A data field base class.
     */
    class LIBLOGICALACCESS_API DataField : public XmlSerializable
#ifndef SWIG
	, public std::enable_shared_from_this < DataField >
#endif        
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
         * \brief Set the field name.
         * \param name The field name.
         */
        void setName(const std::string& name);

        /**
         * \brief Get the field name.
         * \return The field name.
         */
        std::string getName() const;

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         * \param pos The first position bit. Will contain the position bit after the field.
         */
        virtual void getLinearData(void* data, size_t dataLengthBytes, unsigned int* pos) const = 0;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         * \param pos The first position bit. Will contain the position bit after the field.
         */
        virtual void setLinearData(const void* data, size_t dataLengthBytes, unsigned int* pos) = 0;

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
	    void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
	    void unSerialize(boost::property_tree::ptree& node) override;

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
