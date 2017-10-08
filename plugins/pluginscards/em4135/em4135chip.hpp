/**
 * \file em4135chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EM4135 chip.
 */

#ifndef LOGICALACCESS_EM4135CHIP_HPP
#define LOGICALACCESS_EM4135CHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_EM4135			"EM4135"
    /**
     * \brief The EM4135 base chip class.
     */
    class LIBLOGICALACCESS_API EM4135Chip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        EM4135Chip();

        /**
         * \brief Destructor.
         */
        virtual ~EM4135Chip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
	    std::string getGenericCardType() const override { return CHIP_EM4135; }

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
	    std::shared_ptr<LocationNode> getRootLocationNode() override;
    };
}

#endif