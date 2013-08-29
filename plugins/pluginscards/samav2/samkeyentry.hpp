/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_SAMKEYENTRY_HPP
#define LOGICALACCESS_SAMKEYENTRY_HPP

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
		char keyVa;
		char keyVb;
		char keyVc;
		char df_aid_keyno;
		char key_no_v_cek;
		char refkeykuc;
		char updateset;
		char keyversionsentseparatly;
	}				KeyEntryUpdateSettings;

	typedef struct s_SET
	{
		char dumpsessionkey;
		char allowcrypto;
		char keepIV;
		char keytype[3];
		char rfu[2];
		char hightcommandsecurity;
		char disablekeyentry;
		char hostauthenticationafterreset;
		char disablewritekeytopicc;
		char disabledecryption;
		char disableencryption;
		char disableverifymac;
		char disablegeneratemac;
	}		 		SET;

	typedef struct  s_KeyEntryInformation
	{
		unsigned char desfireAid[3];
		unsigned char desfirekeyno;
		unsigned char cekno;
		unsigned char cekv;
		unsigned char kuc;
		unsigned char set[2];
		unsigned char vera;
		unsigned char verb;
		unsigned char verc;
	}				KeyEntryInformation;

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
	 * \brief A DESFire Key class.
	 */
	class LIBLOGICALACCESS_API SAMKeyEntry
	{
		public:

			/**
			 * \brief Build an empty DESFire key.
			 */
			SAMKeyEntry();

			/**
			 * \brief Destructor.
			 */
			~SAMKeyEntry();

			/**
			 * \brief Build a DESFire key given a string representation of it.
			 * \param str The string representation.
			 */
			SAMKeyEntry(const std::string& str, const std::string& str1 = "", const std::string& str2 = "");

			/**
			 * \brief Build a DESFire key given a buffer.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 */
			SAMKeyEntry(const void** buf, size_t buflen, char numberkey);

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
			void setKeyType(SAMKeyType keyType) { d_keyType = keyType; };

			/**
			 * \brief Get the key type.
			 * \return The key type.
			 */
			SAMKeyType getKeyType() const { return d_keyType; };

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
			 * \param ai DESFire key to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const SAMKeyEntry& key) const;

			/**
			 * \brief Inequality operator
			 * \param ai DESFire key to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const SAMKeyEntry& key) const { return !operator==(key); };

			/**
			 * \brief Get the DESFire Key Type in string format.
			 * \return The key type in string.
			 */
			static std::string SAMKeyEntryTypeStr(SAMKeyType t);

			size_t getSingleLength() const;

			void setSET(const SET& t);
			void setSET(unsigned char *t) { memcpy(d_keyentryinformation.set, t, sizeof(*t)); };
			SET getSETStruct();

			unsigned char getUpdateMask() { return d_updatemask; };
			void setUpdateMask(unsigned char c) { d_updatemask = c; };

			KeyEntryUpdateSettings getUpdateSettings();
			void setUpdateSettings(const KeyEntryUpdateSettings& t);

			KeyEntryInformation &getKeyEntryInformation() { return d_keyentryinformation; } ;
			void setKeyEntryInformation(const KeyEntryInformation& t) { d_keyentryinformation = t; };

			void setKeyTypeFromSET();
			void setSETKeyTypeFromKeyType();

		private:

			/**
			 * \brief The key bytes;
			 */
			unsigned char* d_key;

			/**
			 * \brief Diversify the key on the card.
			 */
			bool d_diversify;

			/**
			 * \brief The DESFire key type.
			 */
			SAMKeyType d_keyType;

			unsigned char d_updatemask;

			KeyEntryInformation d_keyentryinformation;
	};
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */

