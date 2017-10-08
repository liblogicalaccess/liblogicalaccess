/**
 * \file desfirelocation.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFireLocation.
 */

#ifndef LOGICALACCESS_DESFIRELOCATION_HPP
#define LOGICALACCESS_DESFIRELOCATION_HPP

#include "logicalaccess/cards/location.hpp"
#include "desfirekey.hpp"

namespace logicalaccess
{
    /**
     * \brief The file communication mode (or encryption mode)
     */
    typedef enum
    {
        CM_PLAIN = 0x00,
        CM_MAC = 0x01,
        CM_ENCRYPT = 0x03,
        CM_UNKNOWN = 0xFF
    }EncryptionMode;

    /**
     * \brief A DESFire location informations.
     */
    class LIBLOGICALACCESS_API DESFireLocation : public Location
    {
    public:
        /**
         * \brief Constructor.
         */
        DESFireLocation();

        /**
         * \brief Destructor.
         */
        virtual ~DESFireLocation();

        /**
         * \brief Convert an Application ID buffer into 32 bits.
         * \param aid The Application ID buffer.
         * \return The Application ID in 32 bits.
         */
        static unsigned int convertAidToUInt(const ByteVector& aid);

        /**
         * \brief Convert an Application ID 32 bits into a buffer.
         * \param i The Application ID in 32 bits.
         * \param aid The Application ID buffer.
         */
        static void convertUIntToAid(unsigned int i, ByteVector& aid);

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

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
	    std::string getDefaultXmlNodeName() const override;

        /**
         * \brief Get the card type for this location.
         * \return The card type.
         */
	    std::string getCardType() override { return "DESFire"; }

        /**
         * \brief Equality operator
         * \param location Location to compare.
         * \return True if equals, false otherwise.
         */
	    bool operator==(const Location& location) const override;

	    /**
         * \brief The application ID.
         */
        unsigned int aid;

        /**
         * \brief The file ID.
         */
        unsigned char file;

        /**
         * \brief The byte offset.
         */
        unsigned int byte_;

        /**
         * \brief The file security level
         */
        EncryptionMode securityLevel;
    };
}

#endif /* LOGICALACCESS_DESFIRELOCATION_HPP */