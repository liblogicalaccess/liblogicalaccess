/**
 * \file mifareplussl1profile.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 card profiles.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL1_HPP
#define LOGICALACCESS_MIFAREPLUSSL1_HPP

#include "mifareplusprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The MifarePlusSL1 base profile class.
     */
    class LIBLOGICALACCESS_API MifarePlusSL1Profile : public MifarePlusProfile
    {
    public:

        /**
         * \brief Constructor.
         */
        MifarePlusSL1Profile();

        /**
         * \brief Destructor.
         */
        virtual ~MifarePlusSL1Profile();

        /**
         * \brief Get one of the Mifare keys of this profile.
         * \param index The index of the key, from 0 to 15.
         * \param keytype The key type.
         * \return The specified mifare key or a null key if either index or keytype is invalid.
         */
        virtual boost::shared_ptr<MifarePlusKey> getKey(int index, MifarePlusKeyType keytype) const;

        /**
         * \brief Set one of the Mifare keys of this profile.
         * \param index The index of the key to set, from 0 to 15.
         * \param keytype The key type.
         * \param key The value of the key.
         * \warning setKey() automatically calls setMifareKeyUsage(index, keytype, true).
         */
        virtual void setKey(int index, MifarePlusKeyType keytype, boost::shared_ptr<MifarePlusKey> key);

        /**
         * \brief Clear all keys in memory.
         */
        virtual void clearKeys();

        /**
         * \brief Set default keys for a sector.
         * \param index The sector index.
         */
        virtual void setDefaultKeysAt(int index);

        /**
         * \brief Get one of the Mifare keys usage.
         * \param index The index of the key
         * \param keytype The keytype.
         * \return true if the key is used, false otherwise.
         */
        virtual bool getKeyUsage(int index, MifarePlusKeyType keytype) const;

        /**
         * \brief Set one of the Mifare keys of this profile.
         * \param index The index of the key to set
         * \param keytype The key type.
         * \param used True if the key is used, false otherwise.
         */
        virtual void setKeyUsage(int index, MifarePlusKeyType keytype, bool used);

        /**
         * \brief Set Mifare key at a specific location.
         * \param location The key's location.
         * \param AccessInfo The key's informations.
         */
        virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo);

        /**
         * \brief Set default keys for the mifare card in memory.
         */
        virtual void setDefaultKeys();

        /**
         * \brief Create default Mifare access informations.
         * \return Default Mifare access informations.
         */
        virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

    protected:

        /**
         * \brief The real keys, + 2 for new keys.
         */
        boost::shared_ptr<MifarePlusKey>* d_sectorKeys;
        boost::shared_ptr<MifarePlusKey> d_switchL2Key;
        boost::shared_ptr<MifarePlusKey> d_switchL3Key;
        boost::shared_ptr<MifarePlusKey> d_SL1AesKey;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUS_HPP */