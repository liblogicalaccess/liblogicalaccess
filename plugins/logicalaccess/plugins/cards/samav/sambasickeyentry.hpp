/**
 * \file sambasickeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief SAMBasicKeyEntry header.
 */

#ifndef LOGICALACCESS_SAMBASICKEYENTRY_HPP
#define LOGICALACCESS_SAMBASICKEYENTRY_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/samav/lla_cards_samav_api.hpp>

/**
 * \brief The total buffer size for keys on a key entry
 */
#define SAM_KEY_BUFFER_SIZE 48

/**
 * \brief The 128-bit key size
 */
#define SAM_KEY_SIZE_128 16
/**
 * \brief The 192-bit key size
 */
#define SAM_KEY_SIZE_192 24
/**
 * \brief The 192-bit key size
 */
#define SAM_KEY_SIZE_256 32

namespace logicalaccess
{
/**
 * \brief The SAM key type.
 */
typedef enum {
    SAM_KEY_DES = 0x00,
    SAM_KEY_MIFARE = 0x10,
    SAM_KEY_3K3DES = 0x18,
    SAM_KEY_AES128 = 0x20,
	SAM_KEY_AES192 = 0x28,
    SAM_KEY_AES256 = 0x38
} SAMKeyType;

typedef struct s_KeyEntryUpdateSettings
{
    unsigned char keyVa;
    unsigned char keyVb;
    unsigned char keyVc;
    unsigned char df_aid_keyno;
    unsigned char key_no_v_cek;
    unsigned char refkeykuc;
    unsigned char updateset;
    unsigned char keyversionsentseparatly;
} KeyEntryUpdateSettings;

typedef struct s_changeKeyInfo
{
    unsigned char desfireNumber;
    bool isMasterKey;
    bool oldKeyInvolvement;
    unsigned char currentKeySlotNo;
    unsigned char currentKeySlotV;
    unsigned char newKeySlotNo;
    unsigned char newKeySlotV;
    bool useChangeKeyEV2;
    unsigned char keysetNumber;
} ChangeKeyInfo;

typedef enum { NO_DIV = 0, SAMAV1, SAMAV2 } NXPKeyDiversificationType;

typedef struct s_changeKeyDiversification
{
    NXPKeyDiversificationType divType;
    unsigned char *divInput;
    unsigned char divInputSize;
    unsigned char diversifyCurrent;
    unsigned char diversifyNew;
} ChangeKeyDiversification;

/**
 * \brief A SAMBasicKeyEntry Key class.
 */
class LLA_CARDS_SAMAV_API SAMBasicKeyEntry
{
  public:
    /**
     * \brief Destructor.
     */
    virtual ~SAMBasicKeyEntry();

    /**
     * \brief Get the key length.
     * \return The key length.
     */
    size_t getLength() const;

    /**
     * \brief Get the number of keys.
     * \return The number of keys.
     */
    unsigned char getKeyNb() const;

    /**
     * \brief Get the keys data.
     * \return The keys data.
     */
    std::vector<ByteVector> getKeysData() const;

    /**
     * \brief Set the keys data.
     */
    virtual void setKeysData(std::vector<ByteVector> keys, SAMKeyType type);

    /**
     * \brief Get the key data.
     * \return The key data.
     */
    const unsigned char *getData() const
    {
        return d_key;
    }

    /**
     * \brief Set if the key is diversified on the card.
     * \param diversify True if the key is diversified on the card, false otherwise.
     */
    void setDiversify(bool diversify)
    {
        d_diversify = diversify;
    }

    /**
     * \brief Get if the key is diversified on the card.
     * \return True if the key is diversified on the card, false otherwise.
     */
    bool getDiversify() const
    {
        return d_diversify;
    }

    /**
     * \brief Set the key type.
     * \param keyType The key type.
     */
    void setKeyType(SAMKeyType keyType)
    {
        d_keyType = keyType;
    } // TODO DELETE IT

    /**
     * \brief Get the key type.
     * \return The key type.
     */
    SAMKeyType getKeyType() const
    {
        return d_keyType;
    } // TODO DELETE IT

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    virtual void serialize(boost::property_tree::ptree &parentNode);

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    virtual void unSerialize(boost::property_tree::ptree &node);

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
    virtual bool operator==(const SAMBasicKeyEntry &key) const;

    /**
     * \brief Inequality operator
     * \param ai SAM key to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const SAMBasicKeyEntry &key) const
    {
        return !operator==(key);
    }

    /**
     * \brief Get the SAM Key Type in string format.
     * \return The key type in string.
     */
    static std::string SAMKeyEntryTypeStr(SAMKeyType t);

    size_t getSingleLength() const;

    unsigned char getUpdateMask() const
    {
        return d_updatemask;
    }
    void setUpdateMask(unsigned char c)
    {
        d_updatemask = c;
    }
	
	static unsigned char getUpdateMask(const KeyEntryUpdateSettings &t);

    KeyEntryUpdateSettings getUpdateSettings();
    void setUpdateSettings(const KeyEntryUpdateSettings &t);

  protected:
    SAMBasicKeyEntry();

    SAMBasicKeyEntry(const SAMBasicKeyEntry &copy);

    SAMBasicKeyEntry(const std::string &str, const std::string &str1,
                     const std::string &str2);

    SAMBasicKeyEntry(const void **buf, size_t buflen, char numberkey);

    /**
     * \brief The key bytes;
     */
    unsigned char d_key[SAM_KEY_BUFFER_SIZE];

    /**
     * \brief Diversify the key on the card.
     */
    bool d_diversify;

    /**
     * \brief The SAM key type.
     */
    SAMKeyType d_keyType; // TODO DELETE IT

    unsigned char d_updatemask;
};
}

#endif /* LOGICALACCESS_SAMBASICKEYENTRY_HPP */