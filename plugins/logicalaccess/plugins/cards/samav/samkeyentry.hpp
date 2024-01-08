/**
 * \file samkeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief samkeyentry header.
 */

#ifndef LOGICALACCESS_SAMKEYENTRY_HPP
#define LOGICALACCESS_SAMKEYENTRY_HPP

#include <cstring>
#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/samav/sambasickeyentry.hpp>

namespace logicalaccess
{
typedef struct s_SETAV1
{
    unsigned char dumpsessionkey;
    unsigned char allowcrypto;
    unsigned char keepIV;
    unsigned char keytype[3];
    unsigned char rfu[2];
    unsigned char hightcommandsecurity;
    unsigned char disablekeyentry;
    unsigned char hostauthenticationafterreset;
    unsigned char disablewritekeytopicc;
    unsigned char disabledecryption;
    unsigned char disableencryption;
    unsigned char disableverifymac;
    unsigned char disablegeneratemac;
} SETAV1;

typedef struct s_SETAV2
{
    unsigned char dumpsessionkey;
    unsigned char allowcrypto;
    unsigned char keepIV;
    unsigned char keytype[3];
    unsigned char rfu[2];
    unsigned char authkey;
    unsigned char disablekeyentry;
    unsigned char lockkey;
    unsigned char disablewritekeytopicc;
    unsigned char disabledecryption;
    unsigned char disableencryption;
    unsigned char disableverifymac;
    unsigned char disablegeneratemac;
} SETAV2;

typedef struct s_EXTSET
{
    unsigned char keyclass[3];
    unsigned char dumpsecretkey;
    unsigned char diversifieduse;
    unsigned char reservedforperso;
    unsigned char rfu;
} ExtSETStruct;

typedef struct s_KeyEntryAV1Information
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
} KeyEntryAV1Information;

typedef struct s_KeyEntryAV2Information
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
    unsigned char ExtSET;
} KeyEntryAV2Information;

/**
 * \brief A SAMKeyEntry class.
 */
template <typename T, typename S>
class LLA_CARDS_SAMAV_API SAMKeyEntry : public SAMBasicKeyEntry
{
  public:
    SAMKeyEntry()
        : SAMBasicKeyEntry()
    {
        memset(&d_keyentryinformation, 0x00, sizeof(d_keyentryinformation));
    }

    explicit SAMKeyEntry(const std::string &str, const std::string &str1 = "",
                         const std::string &str2 = "")
        : SAMBasicKeyEntry(str, str1, str2)
    {
        memset(&d_keyentryinformation, 0x00, sizeof(d_keyentryinformation));
    }

    SAMKeyEntry(const void **buf, size_t buflen, char numberkey)
        : SAMBasicKeyEntry(buf, buflen, numberkey)
    {
        memset(&d_keyentryinformation, 0x00, sizeof(d_keyentryinformation));
    }

    /**
     * \brief Destructor.
     */
    ~SAMKeyEntry()
    {
    }

    /**
     * \brief Inequality operator
     * \param ai SAMKeyEntry key to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const SAMKeyEntry &key) const
    {
        return !operator==(key);
    }

    void setSET(unsigned char *t)
    {
        memcpy(d_keyentryinformation.set, t, sizeof(*t));
    }

    S getSETStruct()
    {
        S set;
        char *x = (char *)&set;

        unsigned char set_save[2];
        unsigned char j = 0;

        memcpy(set_save, d_keyentryinformation.set, 2);

        for (char i = 7; i >= 0; --i)
        {
            if ((set_save[j] & 0x80) == 0x80)
                x[i + j * 8] = 1;
            else
                x[i + j * 8] = 0;
            set_save[j]      = set_save[j] << 1;
            if (i == 0 && j == 0)
            {
                i = 8;
                j++;
            }
        }

        return set;
    }

    void setSET(const S &t)
    {
        char *x = (char *)&t;
        memset(d_keyentryinformation.set, 0, 2);
        unsigned char j = 0;
        for (char i = 7; i >= 0; --i)
        {
            d_keyentryinformation.set[j] += (char)x[i + j * 8];
            if (i == 0 && j == 0)
            {
                i = 8;
                j++;
            }
            else if (i != 0)
                d_keyentryinformation.set[j] = d_keyentryinformation.set[j] << 1;
        }
    }

    T &getKeyEntryInformation()
    {
        return d_keyentryinformation;
    }
    void setKeyEntryInformation(const T &t)
    {
        d_keyentryinformation = t;
    }

    void setKeyTypeFromSET()
    {
        unsigned char keytype = 0x38 & d_keyentryinformation.set[0];
        size_t oldsize        = getLength();
		d_keyType = static_cast<SAMKeyType>(keytype);

        unsigned char *tmp = new unsigned char[getLength()];
        if (getLength() < oldsize)
            oldsize = getLength();
        memcpy(tmp, &*d_key, oldsize);
        delete d_key;
        d_key = tmp;
    }

    void setSETKeyTypeFromKeyType()
    {
        d_keyentryinformation.set[0] -= 0x38 & d_keyentryinformation.set[0];
        d_keyentryinformation.set[0] |= static_cast<unsigned char>(d_keyType);
    }

    void setKeysData(std::vector<ByteVector> keys, SAMKeyType type) override
    {
        SAMBasicKeyEntry::setKeysData(keys, type);
        setSETKeyTypeFromKeyType();
    }

  private:
    T d_keyentryinformation;
};
}

#endif /* LOGICALACCESS_SAMKEYENTRY_HPP */