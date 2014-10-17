/**
 * \file desfireev1profile.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 card profiles.
 */

#ifndef LOGICALACCESS_DESFIREEV1_HPP
#define LOGICALACCESS_DESFIREEV1_HPP

#include "desfireprofile.hpp"
#include "desfireev1location.hpp"

namespace logicalaccess
{
    /**
     * \brief The DESFire EV1 base profile class.
     */
    class LIBLOGICALACCESS_API DESFireEV1Profile : public DESFireProfile
    {
    public:

        /**
         * \brief Constructor.
         */
        DESFireEV1Profile();

        /**
         * \brief Destructor.
         */
        virtual ~DESFireEV1Profile();

        /**
         * \brief Set default keys for the type card in memory at a specific location.
         */
        virtual void setDefaultKeysAt(std::shared_ptr<Location> location);

        /**
         * \brief Create default DESFire location.
         * \return Default DESFire location.
         */
        virtual std::shared_ptr<Location> createLocation() const;
    };
}

#endif /* LOGICALACCESS_DESFIREEV1_HPP */