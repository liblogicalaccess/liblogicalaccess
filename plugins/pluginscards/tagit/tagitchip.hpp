/**
 * \file tagitchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Tag-It chip.
 */

#ifndef LOGICALACCESS_TAGITCHIP_HPP
#define LOGICALACCESS_TAGITCHIP_HPP

#include "../iso15693/iso15693chip.hpp"
#include "tagitcommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define	CHIP_TAGIT		"TagIt"

    /**
     * \brief The Tag-It base chip class.
     */
    class LIBLOGICALACCESS_API TagItChip : public ISO15693Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        TagItChip();

        /**
         * \brief Destructor.
         */
        virtual ~TagItChip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
	    std::shared_ptr<LocationNode> getRootLocationNode() override;
    };
}

#endif