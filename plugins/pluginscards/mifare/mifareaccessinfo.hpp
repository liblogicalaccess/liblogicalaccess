/**
 * \file mifareaccessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief MifareAccessInfo.
 */

#ifndef LOGICALACCESS_MIFAREACCESSINFO_HPP
#define LOGICALACCESS_MIFAREACCESSINFO_HPP

#include "logicalaccess/cards/accessinfo.hpp"
#include "mifarekey.hpp"

namespace logicalaccess
{
    /**
     * \brief Mifare Key Type.
     */
    typedef enum { KT_KEY_A = 0x60, KT_KEY_B = 0x61 } MifareKeyType;

    LIBLOGICALACCESS_API std::ostream &operator<<(std::ostream &s, const MifareKeyType &k);

    /**
     * \brief A Mifare access informations.
     */
    class LIBLOGICALACCESS_API MifareAccessInfo : public AccessInfo
    {
    public:
#ifndef SWIG
        using XmlSerializable::serialize;
        using XmlSerializable::unSerialize;
#endif

        /**
         * \brief Constructor.
         */
        MifareAccessInfo();

        /**
         * \brief Destructor.
         */
        virtual ~MifareAccessInfo();

        /**
         * \brief Generate pseudo-random Mifare access informations.
         */
	    void generateInfos() override;

        /**
         * \brief Get the card type for this access infos.
         * \return The card type.
         */
	    std::string getCardType() const override;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
	    void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
	    void unSerialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
	    std::string getDefaultXmlNodeName() const override;

        /**
         * \brief Equality operator
         * \param ai Access infos to compare.
         * \return True if equals, false otherwise.
         */
	    bool operator==(const AccessInfo& ai) const override;

        /**
         * \brief Block access bits structure.
         */
        struct LIBLOGICALACCESS_API BlockAccessBits
        {
            /**
             * \brief Constructor.
             * \param v1 The first bit value.
             * \param v2 The second bit value.
             * \param v3 The third bit value.
             *
             * Set access bits to (v1, v2, v3).
             */
            BlockAccessBits(bool v1, bool v2, bool v3) : c1(v1), c2(v2), c3(v3) {}

            /**
             * \brief Check if two BlockAccessBits are equal.
             * \param bab A BlockAccessBits to compare with.
             * \return true if the two BlockAccessBits are equal, false otherwise.
             */
	        bool operator==(const BlockAccessBits& bab) const { return ((c1 == bab.c1) && (c2 == bab.c2) && (c3 == bab.c3)); }

            bool c1; /**< \brief The C1 access bit */
            bool c2; /**< \brief The C2 access bit */
            bool c3; /**< \brief The C3 access bit */
        };

        /**
         * \brief Data block access bits structure.
         */
        struct LIBLOGICALACCESS_API DataBlockAccessBits : BlockAccessBits
        {
            /**
             * \brief Constructor.
             *
             * Set access bits to transport configuration. Key A and Key B may serve for any purpose (read, write, increment, etc.).
             */
            DataBlockAccessBits() : BlockAccessBits(false, false, false) {}

			static void* getItem(void* arrayPtr, int i) { return &static_cast<DataBlockAccessBits*>(arrayPtr)[i]; }

			static void setItem(void* arrayPtr, const DataBlockAccessBits* item, int i) { if (item) static_cast<DataBlockAccessBits*>(arrayPtr)[i] = *item; else static_cast<DataBlockAccessBits*>(arrayPtr)[i] = DataBlockAccessBits(); }

        };

        /**
         * \brief Sector trailer block access bits structure.
         */
        struct LIBLOGICALACCESS_API SectorTrailerAccessBits : BlockAccessBits
        {
            /**
             * \brief Constructor.
             *
             * Set access bits to transport configuration. Key A may serve for any purpose (read, write, increment, etc.). Key B is data.
             */
            SectorTrailerAccessBits() : BlockAccessBits(false, false, true) {}

			static void* getItem(void* arrayPtr, int i) { return &static_cast<SectorTrailerAccessBits*>(arrayPtr)[i]; }

			static void setItem(void* arrayPtr, const SectorTrailerAccessBits* item, int i) { if (item) static_cast<SectorTrailerAccessBits*>(arrayPtr)[i] = *item; else static_cast<SectorTrailerAccessBits*>(arrayPtr)[i] = SectorTrailerAccessBits(); }
		};

        /**
         * \brief Sector access bits structure.
         */
        struct LIBLOGICALACCESS_API SectorAccessBits
        {
            /**
             * \brief Export the access bits to a mifare byte array.
             * \return The buffer into which write the data.
             */
            ByteVector toArray() const;

            /**
             * \brief Import the access bits from a mifare byte array.
             * \param buf The buffer containing the data.
             * \return true on success, false otherwise.
             *
             * If the array is not a valid mifare sector access bits array, fromArray() fails and the sector access bits remains unchanged.
             */
            bool fromArray(ByteVector);

            /**
             * \brief Set transport configuration.
             */
            void setTransportConfiguration();

            /**
             * \brief Set A read, B write configuration.
             */
            void setAReadBWriteConfiguration();

            DataBlockAccessBits d_data_blocks_access_bits[3]; /**< \brief The data blocks access bits. */
            SectorTrailerAccessBits d_sector_trailer_access_bits; /**< \brief The sector trailer access bits. */
        };

        /**
         * \brief The key A.
         */
        std::shared_ptr<MifareKey> keyA;

        /**
         * \brief The key B.
         */
        std::shared_ptr<MifareKey> keyB;

        /**
         * \brief MAD is used ?
         */
        bool useMAD;

        /**
         * \brief The MAD A Key.
         */
        std::shared_ptr<MifareKey> madKeyA;

        /**
         * \brief The MAD B Key.
         */
        std::shared_ptr<MifareKey> madKeyB;

        /**
         * \brief The sector access bits.
         */
        SectorAccessBits sab;

        /**
         * \brief The sector GPB.
         */
        unsigned char gpb;

        /**
         * \brief The MAD GPB.
         */
        unsigned char madGPB;
    };
}

#endif /* LOGICALACCESS_MIFAREACCESSINFO_HPP */