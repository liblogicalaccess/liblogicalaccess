/**
 * \file MifarePlusAccessInfo.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusAccessInfo.
 */

#ifndef LOGICALACCESS_MIFAREPLUSACCESSINFO_H
#define LOGICALACCESS_MIFAREPLUSACCESSINFO_H

#include "logicalaccess/Cards/AccessInfo.h"
#include "MifarePlusKey.h"

namespace LOGICALACCESS
{

#define MIFARE_PLUS_AES_KEY_SIZE		0x10
#define MIFARE_PLUS_CRYPTO1_KEY_SIZE	0x06
#define MIFARE_PLUS_BLOCK_SIZE			0x10
#define MIFARE_PLUS_DEFAULT_AESKEY		"0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF"
#define MIFARE_PLUS_DEFAULT_CRYPTO1KEY	"0xFF 0xFF 0xFF 0xFF 0xFF 0xFF"
#define MIFARE_PLUS_MAX_SECTORNB		40

	/**
	 * \brief MifarePlus Key Type.
	 */
	typedef enum	{	KT_KEY_AES_A =			0x40,
					KT_KEY_AES_B =			0x41,
					KT_KEY_CRYPTO1_A =		0x42,
					KT_KEY_CRYPTO1_B =		0x43,
					KT_KEY_ORIGINALITY =	0x80,
					KT_KEY_MASTERCARD =		0x90,
					KT_KEY_CONFIGURATION =	0x91,
					KT_KEY_SWITCHL2 =		0x92,
					KT_KEY_SWITCHL3 =		0x93,
					KT_KEY_AESSL1 =			0x94
					} MifarePlusKeyType;	

	/**
	 * \brief A MifarePlus access informations.
	 */
	class LIBLOGICALACCESS_API MifarePlusAccessInfo : public AccessInfo
	{
		public:
			/**
			 * \brief Constructor.
			 */
			MifarePlusAccessInfo(size_t sectorKeySize);

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusAccessInfo();

			/**
			 * \brief Generate pseudo-random Mifare access informations.
			 */
			virtual void generateInfos();

			/**
			 * \brief Generate pseudo-random Mifare access informations for a given card serial number.
			 * \param csn The card serial number.
			 */
			virtual void generateInfos(const string& csn);

			/**
			 * \brief Export DESFire location informations to a buffer.
			 * \return The data.
			 */
			virtual std::vector<unsigned char> getLinearData() const;

			/**
			 * \brief Import DESFire location informations from a buffer.
			 * \param data The buffer.
			 * \param offset An offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>&, size_t offset = 0);

			/**
			 * \brief Mifare access informations data size.
			 */
			virtual size_t getDataSize();

			/**
			 * \brief Get the card type for this access infos.
			 * \return The card type.
			 */
			virtual std::string getCardType() { return "MifarePlus4K"; }

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Equality operator
			 * \param ai Access infos to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const AccessInfo& ai) const;


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
				BlockAccessBits(bool v1, bool v2, bool v3) : c1(v1), c2(v2), c3(v3) {};

				/**
				 * \brief Check if two BlockAccessBits are equal.
				 * \param bab A BlockAccessBits to compare with.
				 * \return true if the two BlockAccessBits are equal, false otherwise.
				 */
				inline bool operator==(const BlockAccessBits& bab) const { return ((c1 == bab.c1) && (c2 == bab.c2) && (c3 == bab.c3)); };

				bool c1; /**< \brief The C1 access bit */
				bool c2; /**< \brief The C2 access bit */
				bool c3; /**< \brief The C3 access bit */
			};

			/**
			 * \brief Data block access bits structure.
			 */
			struct LIBLOGICALACCESS_API DataBlockAccessBits : public BlockAccessBits
			{
				/**
				 * \brief Constructor.
				 *
				 * Set access bits to transport configuration. Key A and Key B may serve for any purpose (read, write, increment, etc.).
				 */
				DataBlockAccessBits() : BlockAccessBits(false, false, false) {};
			};

			/**
			 * \brief Sector trailer block access bits structure.
			 */
			struct LIBLOGICALACCESS_API SectorTrailerAccessBits : public BlockAccessBits
			{
				/**
				 * \brief Constructor.
				 *
				 * Set access bits to transport configuration. Key A may serve for any purpose (read, write, increment, etc.). Key B is data.
				 */
				SectorTrailerAccessBits() : BlockAccessBits(false, false, true) {};
			};

			/**
			 * \brief Sector access bits structure.
			 */
			struct LIBLOGICALACCESS_API SectorAccessBits
			{
				/**
				 * \brief Export the access bits to a mifare byte array.
				 * \param buf The buffer into which write the data.
				 * \param buflen buf length. buflen must be at least 3 or toArray() will fail.
				 * \return The count of bytes used (shall be 3) on success, 0 otherwise.
				 */
				size_t toArray(void* buf, size_t buflen) const;

				/**
				 * \brief Import the access bits from a mifare byte array.
				 * \param buf The buffer containing the data.
				 * \param buflen The length of buf. Must be at least 3 or fromArray() will fail.
				 * \return true on success, false otherwise.
				 *
				 * If the array is not a valid mifare sector access bits array, fromArray() fails and the sector access bits remains unchanged.
				 */
				bool fromArray(const void* buf, size_t buflen);

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

		protected:

			/**
			 * \brief The sector key size (AES or crypto1).
			 */
			size_t d_sectorKeySize;

		public:

			/**
			 * \brief The key A.
			 */
			boost::shared_ptr<MifarePlusKey> keyA;

			/**
			 * \brief The key B.
			 */
			boost::shared_ptr<MifarePlusKey> keyB;

			/**
			 * \brief The key Originality.
			 */
			boost::shared_ptr<MifarePlusKey> keyOriginality;

			/**
			 * \brief The key Mastercard.
			 */
			boost::shared_ptr<MifarePlusKey> keyMastercard;

			/**
			 * \brief The key Configuration.
			 */
			boost::shared_ptr<MifarePlusKey> keyConfiguration;

			/**
			 * \brief The key Switch level 2.
			 */
			boost::shared_ptr<MifarePlusKey> keySwitchL2;

			/**
			 * \brief The key Switch level 3.
			 */
			boost::shared_ptr<MifarePlusKey> keySwitchL3;

			/**
			 * \brief The key SL1 AES authentification.
			 */
			boost::shared_ptr<MifarePlusKey> keyAuthenticateSL1AES;

			/**
			 * \brief MAD is used ?
			 */
			bool useMAD;

			/**
			 * \brief The MAD A Key.
			 */
			boost::shared_ptr<MifarePlusKey> madKeyA;

			/**
			 * \brief The MAD B Key.
			 */
			boost::shared_ptr<MifarePlusKey> madKeyB;

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

#endif /* LOGICALACCESS_MIFAREPLUSACCESSINFO_H */
