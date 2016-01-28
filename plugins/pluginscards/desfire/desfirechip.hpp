/**
 * \file desfirechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire chip.
 */

#ifndef LOGICALACCESS_DESFIRECHIP_HPP
#define LOGICALACCESS_DESFIRECHIP_HPP

#include "logicalaccess/cards/chip.hpp"
#include "desfirecommands.hpp"
#include "desfireprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_DESFIRE		"DESFire"

    /**
     * \brief The DESFire base chip class.
     */
    class LIBLOGICALACCESS_API DESFireChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        DESFireChip();

        /**
         * \brief Constructor.
         * \param ct The card type.
         */
        DESFireChip(std::string ct);

        /**
         * \brief Destructor.
         */
        virtual ~DESFireChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_DESFIRE; };

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

        /**
         * \brief Get a card service for this card provider.
         * \param serviceType The card service type.
         * \return The card service.
         */
        virtual std::shared_ptr<CardService> getService(CardServiceType serviceType);

        /**
         * \brief Get the DESFire commands.
         * \return The DESFire commands.
         */
        std::shared_ptr<DESFireCommands> getDESFireCommands() { return std::dynamic_pointer_cast<DESFireCommands>(getCommands()); };

        /**
         * \brief Get the DESFire profile.
         * \return The DESFire profile.
         */
        std::shared_ptr<DESFireProfile> getDESFireProfile() { return std::dynamic_pointer_cast<DESFireProfile>(getProfile()); };


        /**
         * Set the flag that tells us wether or now the
         * current chip identifier is real and not random.
         */
        void setHasRealUID(bool enabled)
        {
            has_real_uid_ = enabled;
        }

        /**
         * Does the current chip identifier represents the "real uid" or
         * is it a "random UID" ?
         *
         * This flag is updated when we fetch the real UID for the first time.
         */
        bool hasRealUID() const
        {
            return has_real_uid_;
        }

    protected:
        /**
         * Is random UUID enabled or not ?
         * This is detected when creating the chip object in PCSC Reader.
         */
        bool has_real_uid_;
    };
}

#endif /* LOGICALACCESS_DESFIRECHIP_HPP */