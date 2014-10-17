/**
 * \file desfireprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire card profiles.
 */

#ifndef LOGICALACCESS_DESFIRE_HPP
#define LOGICALACCESS_DESFIRE_HPP

#include "logicalaccess/cards/profile.hpp"
#include "desfireaccessinfo.hpp"
#include "desfirelocation.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The DESFire base profile class.
     */
    class LIBLOGICALACCESS_API DESFireProfile : public Profile
    {
    public:

        /**
         * \brief Constructor.
         */
        DESFireProfile();

        /**
         * \brief Destructor.
         */
        virtual ~DESFireProfile();

        /**
         * \brief Get key from memory.
         * \param aid The Application ID
         * \param keyno The key number
         * \param diversify The diversify buffer, NULL if no diversification is needed
         * \param keydiv The key data, diversified if a diversify buffer is specified.
         * \return True on success, false otherwise.
         */
        bool getKey(size_t aid, unsigned char keyno, std::vector<unsigned char> diversify, std::vector<unsigned char>& keydiv);

        /**
         * \brief Get one of the DESFire keys of this profile.
         * \param aid The application id.
         * \param keyno The key number.
         * \return The specified DESFire key or a null key if params are invalid.
         */
        std::shared_ptr<DESFireKey> getKey(size_t aid, unsigned char keyno) const;

        /**
         * \brief Set one of the DESFire keys of this profile.
         * \param aid Application ID
         * \param keyno The key number to set
         * \param key The value of the key.
         */
        virtual void setKey(size_t aid, unsigned char keyno, std::shared_ptr<DESFireKey> key);

        /**
         * \brief Set default keys for the card in memory.
         */
        virtual void setDefaultKeys();

        /**
         * \brief Set default keys for the type card in memory at a specific location.
         */
        virtual void setDefaultKeysAt(std::shared_ptr<Location> location);

        /**
         * \brief Clear all keys in memory.
         */
        virtual void clearKeys();

        /**
         * \brief Set DESFire key at a specific location.
         * \param location The key's location.
         * \param AccessInfo The key's informations.
         */
        virtual void setKeyAt(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> AccessInfo);

        /**
         * \brief Create default DESFire access informations.
         * \return Default DESFire access informations.
         */
        virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

        /**
         * \brief Create default DESFire location.
         * \return Default DESFire location.
         */
        virtual std::shared_ptr<Location> createLocation() const;

        /**
         * \brief Get the supported format list.
         * \return The format list.
         */
        virtual FormatList getSupportedFormatList();

        /**
         * \brief Get the default key for an algorithm.
         * \param keyType The key algorithm type.
         * \return The default key.
         */
        static std::shared_ptr<DESFireKey> getDefaultKey(DESFireKeyType keyType);

    protected:

        /**
         * \brief Check if a key position exists.
         * \param aid The Application ID
         * \param defvalue The default result value
         * \return The check result.
         */
        bool checkKeyPos(size_t aid, bool defvalue = false) const;

        /**
         * \brief Check if a key position exists.
         * \param aid The Application ID
         * \param keyno The key number
         * \param defvalue The default result value
         * \return The check result.
         */
        bool checkKeyPos(size_t aid, unsigned char keyno, bool defvalue) const;

        /**
         * \brief Get the position in memory.
         * \param aid The Application ID
         * \param pos Will contain the position in memory
         * \return True on success, false otherwise.
         */
        bool getPosAid(size_t aid, size_t* pos = NULL) const;

        /**
         * \brief Add Application ID position.
         * \param aid The Application ID
         * \return The Application ID position.
         */
        size_t addPosAid(size_t aid);

        /**
         * \brief Get one of the DESFire keys usage.
         * \param index The index of the key in memory
         * \return true if the key is used, false otherwise.
         */
        bool getKeyUsage(size_t index) const;

        /**
         * \brief Set one of the DESFire keys of this profile.
         * \param index The index of the key  in memory to set
         * \param used True if the key is used, false otherwise.
         */
        void setKeyUsage(size_t index, bool used);

        /**
         * \brief The 393 real keys.
         *		0		:	MasterKey Card Maintenance
         *		1		:	MasterKey Application 1 Maintenance
         *		2 - 15	:	3(DES) Application 1 keys
         *		16		:	MasterKey Application 2 Maintenance
         *		17 - 30	:	3(DES) Application 2 keys
         *		.............
         */
        std::shared_ptr<DESFireKey> d_key[393];

        /**
         * \brief The Application ID used.
         */
        size_t d_aids[28];

        /**
         * \brief Number of Application ID used.
         */
        size_t d_nbAids;
    };
}

#endif /* LOGICALACCESS_DESFIRE_HPP */