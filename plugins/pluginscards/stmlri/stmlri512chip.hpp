/**
 * \file stmlri512chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STM LRI512 chip.
 */

#ifndef LOGICALACCESS_STMLRI512CHIP_HPP
#define LOGICALACCESS_STMLRI512CHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_STMLRI512	"StmLri512"

    /**
     * \brief The STM LRI512 base chip class.
     */
    class LIBLOGICALACCESS_API StmLri512Chip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        StmLri512Chip();

        /**
         * \brief Destructor.
         */
        virtual ~StmLri512Chip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
	    std::string getGenericCardType() const override { return CHIP_STMLRI512; }

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
	    std::shared_ptr<LocationNode> getRootLocationNode() override;
    };
}

#endif