/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_SAMAV2KEYENTRY_HPP
#define LOGICALACCESS_SAMAV2KEYENTRY_HPP

#include "logicalaccess/key.hpp"


/**
 * \brief The SAM DES key size
 */
#define SAMAV2_DES_KEY_SIZE 16

/**
 * \brief The SAM AES key size
 */
#define SAMAV2_AES_KEY_SIZE 16

/**
 * \brief The SAM max key size
 */
#define SAMAV2_MAXKEY_SIZE 24

namespace logicalaccess
{
	/**
	 * \brief The SAM key type.
	 */
	typedef enum 
	{
		SAMAV2_KEY_DES = 0x00,
		SAMAV2_KEY_3K3DES = 0x40,
		SAMAV2_KEY_AES = 0x80
	}				SAMAV2KeyType;

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
		char	dumpsessionkey;
		char	allowcrypto;
		char	keepIV;
		char	keytype[3];
		char	rfu[2];
		char	authkey;
		char	disablekeyentry;
		char	lockkey;
		char	disablewritekeytopicc;
		char	disabledecryption;
		char	disableencryption;
		char	disableverifymac;
		char	disablegeneratemac;
	}				SET;

	typedef struct  s_KeyEntryInformation
	{
		unsigned char	desfireAid[3];
		unsigned char	desfirekeyno;
		unsigned char	cekno;
		unsigned char	cekv;
		unsigned char	kuc;
		unsigned char	set[2];
		unsigned char	vera;
		unsigned char	verb;
		unsigned char	verc;
	}				KeyEntryInformation;

	/**
	 * \brief A DESFire Key class.
	 */
	class LIBLOGICALACCESS_API SAMAV2KeyEntry
	{
		public:

			/**
			 * \brief Build an empty DESFire key.
			 */
			SAMAV2KeyEntry();

			/**
			 * \brief Build a DESFire key given a string representation of it.
			 * \param str The string representation.
			 */
			SAMAV2KeyEntry(const std::string& str, const std::string& str1 = "", const std::string& str2 = "");

			/**
			 * \brief Build a DESFire key given a buffer.
			 * \param buf The buffer.
			 * \param buflen The buffer length.
			 */
			SAMAV2KeyEntry(const void** buf, size_t buflen, char numberkey);

			/**
			 * \brief Get the key length.
			 * \return The key length.
			 */
			size_t getLength() const;

			/**
			 * \brief Get the key data.
			 * \return The key data.
			 */
			inline const unsigned char* getKeyA() const { return &d_keyA[0]; };
			inline const unsigned char* getKeyB() const { return &d_keyB[0]; };
			inline const unsigned char* getKeyC() const { return &d_keyC[0]; };
			/**
			 * \brief Get the key data.
			 * \return The key data.
			 */
			inline unsigned char* getDataA() { return &d_keyA[0]; };
			inline unsigned char* getDataB() { return &d_keyB[0]; };
			inline unsigned char* getDataC() { return &d_keyC[0]; };

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
			void setKeyType(SAMAV2KeyType keyType) { d_keyType = keyType; };

			/**
			 * \brief Get the key type.
			 * \return The key type.
			 */
			SAMAV2KeyType getKeyType() const { return d_keyType; };

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
			virtual bool operator==(const SAMAV2KeyEntry& key) const;

			/**
			 * \brief Inequality operator
			 * \param ai DESFire key to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const SAMAV2KeyEntry& key) const { return !operator==(key); };

			/**
			 * \brief Get the DESFire Key Type in string format.
			 * \return The key type in string.
			 */
			static std::string SAMAV2KeyEntryTypeStr(SAMAV2KeyType t);

			void	setSET(SET t);
			void	setSET(unsigned char *t) { memcpy(d_keyentryinformation->set, t, sizeof(*t)); };
			boost::shared_ptr<SET> getSETStruct();

			unsigned char	getUpdateMask() { return d_updatemask; };
			void			setUpdateMask(unsigned char c) { d_updatemask = c; };
			boost::shared_ptr<KeyEntryUpdateSettings> getUpdateSettings();
			void	setUpdateSettings(boost::shared_ptr<KeyEntryUpdateSettings> t);

			boost::shared_ptr<KeyEntryInformation> getKeyEntryInformation() { return d_keyentryinformation; } ;

			void		setKeyEntryInformation(boost::shared_ptr<KeyEntryInformation> t) { d_keyentryinformation = t; };

		private:

			/**
			 * \brief The key bytes;
			 */
			std::vector<unsigned char> d_keyA;
			std::vector<unsigned char> d_keyB;
			std::vector<unsigned char> d_keyC;

			/**
			 * \brief Diversify the key on the card.
			 */
			bool d_diversify;

			/**
			 * \brief The DESFire key type.
			 */
			SAMAV2KeyType d_keyType;

			unsigned char d_updatemask;

			boost::shared_ptr<KeyEntryInformation> d_keyentryinformation;
	};
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */

