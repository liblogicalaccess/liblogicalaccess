/**
 * \file generictagidondemandchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand chip.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDCHIP_HPP
#define LOGICALACCESS_GENERICTAGIDONDEMANDCHIP_HPP

#include "generictag/generictagchip.hpp"

namespace logicalaccess
{
    /**
     * \brief The Generic Tag IdOnDemand commands class.
     */
    class LIBLOGICALACCESS_API GenericTagIdOnDemandChip : public GenericTagChip
    {
    public:

        /**
         * \brief Constructor.
         */
        GenericTagIdOnDemandChip();

        /**
         * \brief Destructor.
         */
        ~GenericTagIdOnDemandChip();

        /**
         * \brief Get a card service for this card provider.
         * \param serviceType The card service type.
         * \return The card service.
         */
        virtual std::shared_ptr<CardService> getService(CardServiceType serviceType);
    };
}

#endif