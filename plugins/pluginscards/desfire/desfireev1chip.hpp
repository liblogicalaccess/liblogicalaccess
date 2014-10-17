/**
 * \file desfireev1chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 chip.
 */

#ifndef LOGICALACCESS_DESFIREEV1CHIP_HPP
#define LOGICALACCESS_DESFIREEV1CHIP_HPP

#include "desfirechip.hpp"
#include "desfireev1commands.hpp"
#include "desfireev1profile.hpp"

namespace logicalaccess
{
#define	CHIP_DESFIRE_EV1		"DESFireEV1"

    /**
     * \brief The DESFire EV1 base chip class.
     */
    class LIBLOGICALACCESS_API DESFireEV1Chip : public DESFireChip
    {
    public:

        /**
         * \brief Constructor.
         */
        DESFireEV1Chip();

        /**
         * \brief Destructor.
         */
        virtual ~DESFireEV1Chip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

        /**
         * \brief Get the application location information.
         * \return The location.
         */
        virtual std::shared_ptr<DESFireLocation> getApplicationLocation();

        virtual std::shared_ptr<CardService> getService(CardServiceType serviceType);

        /**
         * \brief Get the DESFire card provider for I/O access.
         * \return The DESFire card provider.
         */
        std::shared_ptr<DESFireEV1Commands> getDESFireEV1Commands() { return std::dynamic_pointer_cast<DESFireEV1Commands>(getCommands()); };

        /**
         * \brief Get the DESFire EV1 profile.
         * \return The DESFire EV1 profile.
         */
        std::shared_ptr<DESFireEV1Profile> getDESFireEV1Profile() { return std::dynamic_pointer_cast<DESFireEV1Profile>(getProfile()); };

    protected:
    };
}

#endif /* LOGICALACCESS_DESFIREEV1CHIP_HPP */