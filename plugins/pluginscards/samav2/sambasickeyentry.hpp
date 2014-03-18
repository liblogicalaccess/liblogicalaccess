/**
 * \file sambasickeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMBasicKeyEntry header.
 */

#ifndef LOGICALACCESS_SAMBASICKEYENTRY_HPP
#define LOGICALACCESS_SAMBASICKEYENTRY_HPP

#include "logicalaccess/key.hpp"


/**
 * \brief The SAM DES key size
 */
#define SAM_DES_KEY_SIZE 16

/**
 * \brief The SAM AES key size
 */
#define SAM_AES_KEY_SIZE 16

/**
 * \brief The SAM max key size
 */
#define SAM_MAXKEY_SIZE 24

namespace logicalaccess
{
	/**
	 * \brief The SAM key type.
	 */
	typedef enum 
	{
		SAM_KEY_DES = 0x00,
		SAM_KEY_3K3DES = 0x0c,
		SAM_KEY_AES = 0x10
	}				SAMKeyType;

	typedef struct  s_KeyEntryUpdateSettings
	{
		unsigned char keyVa;
		unsigned char keyVb;
		unsigned char keyVc;
		unsigned char df_aid_keyno;
		unsigned char key_no_v_cek;
		unsigned char refkeykuc;
		unsigned char updateset;
		unsigned char keyversionsentseparatly;
	}				KeyEntryUpdateSettings;

	typedef struct s_changeKeyInfo
	{
		unsigned char desfireNumber;
		unsigned char isMasterKey;
		unsigned char oldKeyInvolvement;
		unsigned char currentKeyNo;
		unsigned char currentKeyV;
		unsigned char newKeyNo;
		unsigned char newKeyV;
	}				ChangeKeyInfo;

	/**
	 * \brief A SAMBasicKeyEntry Key class.
	 */
	class LIBLOGICALACCESS_API SAMBasicKeyEntry
	{
		public:
			/**
			 * \brief Destructor.
			 */
			~SAMBasicKeyEntry();

			/**
			 * \brief Get the key length.
			 * \return The key length.
			 */
			size_t getLength() const;

			/**
			 * \brief Get the keys data.
			 * \return The keys data.
			 */
			std::vector< std::vector<unsigned char> > getKeysData();

			/**
			 * \brief Set the keys data.
			 */
			void setKeysData(std::vector<std::vector<unsigned char> > keys, SAMKeyType type);

			/**
			 * \brief Get the key data.
			 * \return The key data.
			 */
			inline unsigned char *getData() { return d_key; };

			/**
			 * \brief Set if the key is diversified on the card.
			 * \param diversify True if the key is diversified on the card, false otherwise.
			 */
			void setDiversify(bool diversify) { d_diversify = diversify; };

			/**
			 * \brief Get if the key is diversified on the card.
			 * \return True if the key is diversified on the card, false otherwise.
			 */
			bool getDiversify() const { return d_diversify; };

			/**
			 * \brief Set the key type.
			 * \param keyType The key type.
			 */
			void setKeyType(SAMKeyType keyType) { d_keyType = keyType; }; //TODO DELETE IT

			/**
			 * \brief Get the key type.
			 * \return The key type.
			 */
			SAMKeyType getKeyType() const { return d_keyType; }; //TODO DELETE IT

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
			 * \brief Equality operator
			 * \param ai SAM key to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const SAMBasicKeyEntry& key) const;

			/**
			 * \brief Inequality operator
			 * \param ai SAM key to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const SAMBasicKeyEntry& key) const { return !operator==(key); };

			/**
			 * \brief Get the SAM Key Type in string format.
			 * \return The key type in string.
			 */
			static std::string SAMKeyEntryTypeStr(SAMKeyType t);

			size_t getSingleLength() const;

			unsigned char getUpdateMask() { return d_updatemask; };
			void setUpdateMask(unsigned char c) { d_updatemask = c; };

			KeyEntryUpdateSettings getUpdateSettings();
			void setUpdateSettings(const KeyEntryUpdateSettings& t);

		protected:

			SAMBasicKeyEntry();

			SAMBasicKeyEntry(const std::string& str, const std::string& str1, const std::string& str2);

			SAMBasicKeyEntry(const void** buf, size_t buflen, char numberkey);
			
			/**
			 * \brief The key bytes;
			 */
			unsigned char* d_key;

			/**
			 * \brief Diversify the key on the card.
			 */
			bool d_diversify;

			/**
			 * \brief The SAM key type.
			 */
			SAMKeyType d_keyType; //TODO DELETE IT

			unsigned char d_updatemask;
	};
}

#endif /* LOGICALACCESS_SAMBASICKEYENTRY_HPP */

