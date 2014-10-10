/**
 * \file mifareultralightcprofile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C card profiles.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTC_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTC_HPP

#include "mifareultralightprofile.hpp"
#include "logicalaccess/cards/tripledeskey.hpp"

namespace logicalaccess
{
    /**
     * \brief The Mifare Ultralight C base profile class.
     */
    class LIBLOGICALACCESS_API MifareUltralightCProfile : public MifareUltralightProfile
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightCProfile();

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCProfile();

        /**
         * \brief Set default keys for the mifare card in memory.
         */
        virtual void setDefaultKeys();

        /**
         * \brief Clear all keys in memory.
         */
        virtual void clearKeys();

        /**
         * \brief Set key at a specific location.
         * \param location The key's location.
         * \param AccessInfo The key's informations.
         */
        virtual void setKeyAt(boost::shared_ptr<Location> location, boost::shared_ptr<AccessInfo> AccessInfo);

        /**
         * \brief Set the Mifare Ultralight C authentication key.
         * \param key The authentication key.
         */
        void setKey(boost::shared_ptr<TripleDESKey> key);

        /**
         * \brief Get the Mifare Ultralight C authentication key.
         * \return The authentication key.
         */
        boost::shared_ptr<TripleDESKey> getKey() const;

        /**
         * \brief Create default access informations.
         * \return Default access informations. Always null.
         */
        virtual boost::shared_ptr<AccessInfo> createAccessInfo() const;

    protected:

        boost::shared_ptr<TripleDESKey> d_key;
    };
}

#endif