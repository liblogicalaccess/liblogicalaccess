/**
 * \file format.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Format.
 */

#ifndef LOGICALACCESS_FORMAT_HPP
#define LOGICALACCESS_FORMAT_HPP

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "logicalaccess/xmlserializable.hpp"
#include "logicalaccess/services/accesscontrol/encodings/datarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/datatype.hpp"

#include "logicalaccess/logs.hpp"

#include "logicalaccess/services/accesscontrol/formats/customformat/datafield.hpp"
#include <list>

namespace logicalaccess
{
    /**
     * \brief Format type
     */
    typedef enum {
        FT_UNKNOWN = 0x00,
        FT_WIEGAND26 = 0x02,
        FT_WIEGAND34 = 0x03,
        FT_WIEGAND34FACILITY = 0x04,
        FT_WIEGAND37 = 0x05,
        FT_WIEGAND37FACILITY = 0x06,
        FT_WIEGANDFLEXIBLE = 0x07, /** Removed, use Custom now */
        FT_ASCII = 0x08,
        FT_CORPORATE1000 = 0x09,
        FT_DATACLOCK = 0x0A,
        FT_FASCN200BIT = 0x0B,
        FT_HIDHONEYWELL = 0x0C,
        FT_GETRONIK40BIT = 0x0D,
        FT_BARIUM_FERRITE_PCSC = 0x0E,
        FT_CUSTOM = 0x10,
        FT_RAW = 0xFF
    } FormatType;

    bool FieldSortPredicate(const boost::shared_ptr<DataField>& lhs, const boost::shared_ptr<DataField>& rhs);

    /**
     * \brief A format.
     */
    class LIBLOGICALACCESS_API Format : boost::noncopyable, public XmlSerializable
    {
    public:
        /**
         * \brief Constructor.
         *
         * Create a Format.
         */
        Format();

        /**
         * \brief Get data length in bits.
         * \return The data length in bits.
         */
        virtual unsigned int getDataLength() const = 0;

        /**
         * \brief Get the format name.
         * \return The format name.
         */
        virtual string getName() const = 0;

        /**
         * \brief Get linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
        virtual void getLinearData(void* data, size_t dataLengthBytes) const = 0;

        /**
         * \brief Set linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length of data in bytes
         */
        virtual void setLinearData(const void* data, size_t dataLengthBytes) = 0;

        /**
         * \brief Get skeleton linear data.
         * \param data Where to put data
         * \param dataLengthBytes Length in byte of data
         */
        virtual size_t getSkeletonLinearData(void* data, size_t dataLengthBytes) const = 0;

        /**
         * \brief Set skeleton linear data.
         * \param data Where to get data
         * \param dataLengthBytes Length in byte of data
         */
        virtual void setSkeletonLinearData(const void* data, size_t dataLengthBytes) = 0;

        /**
         * \brief Get the format type.
         * \return The format type.
         */
        virtual FormatType getType() const = 0;

        /**
         * \brief Get a new format instance from a format type.
         * \param type The format type.
         * \return The new format instance.
         */
        static boost::shared_ptr<Format> getByFormatType(FormatType type);

        /**
         * \brief Get values field list.
         * \return The values field list.
         */
        virtual std::vector<std::string> getValuesFieldList() const;

        /**
         * \brief Get the field length.
         * \param field The field.
         * \return The field length.
         */
        virtual unsigned int getFieldLength(const std::string& field) const;

        /**
         * \brief Check the current format skeleton with another format.
         * \param format The format to check.
         * \return True on success, false otherwise.
         */
        virtual bool checkSkeleton(boost::shared_ptr<Format> format) const = 0;

        /**
         * \brief Calculate parity for a block of data.
         * \param data Data block
         * \param dataLengthBytes Length of data in bytes
         * \param parityType Parity type
         * \param positions List of positions to calculate parity
         * \param nbPositions Number of parity in the list
         * \return The parity.
         */
        static unsigned char calculateParity(const void* data, size_t dataLengthBytes, ParityType parityType, unsigned int* positions, size_t nbPositions);

        /**
         * \brief Get the identifier.
         * \return The identifier.
         */
        virtual std::vector<unsigned char> getIdentifier();

        /**
         * \brief Get the format field list.
         * \return The field list.
         */
        virtual std::list<boost::shared_ptr<DataField> > getFieldList();

        /**
         * \brief Get the format field list.
         * \param fields The field list.
         */
        virtual void setFieldList(std::list<boost::shared_ptr<DataField> > fields);

        /**
         * \brief Get the field object from name.
         */
        boost::shared_ptr<DataField> getFieldFromName(std::string field) const;

    protected:

        /**
         * \brief The field list.
         */
        std::list<boost::shared_ptr<DataField> > d_fieldList;
    };
}

#endif /* LOGICALACCESS_FORMAT_HPP */