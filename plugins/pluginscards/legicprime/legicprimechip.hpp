/**
 * \file legicprimechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Legic Prime chip.
 */

#ifndef LOGICALACCESS_LEGICPRIMECHIP_HPP
#define LOGICALACCESS_LEGICPRIMECHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_LEGICPRIME		"LegicPrime"

    /**
     * \brief The Legic Prime base chip class.
     */
    class LIBLOGICALACCESS_API LegicPrimeChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        LegicPrimeChip();

        /**
         * \brief Destructor.
         */
        virtual ~LegicPrimeChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_LEGICPRIME; };

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual boost::shared_ptr<LocationNode> getRootLocationNode();

    protected:
    };
}

#endif