/**
 * \file indalachip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Indala chip.
 */

#ifndef LOGICALACCESS_INDALACHIP_HPP
#define LOGICALACCESS_INDALACHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_INDALA		"Indala"

    /**
     * \brief The Indala base chip class.
     */
    class LIBLOGICALACCESS_API IndalaChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        IndalaChip();

        /**
         * \brief Destructor.
         */
        virtual ~IndalaChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
        virtual std::string getGenericCardType() const { return CHIP_INDALA; };

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

    protected:
    };
}

#endif