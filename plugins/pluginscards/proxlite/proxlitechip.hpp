/**
 * \file proxlitechip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox Lite chip.
 */

#ifndef LOGICALACCESS_PROXLITECHIP_HPP
#define LOGICALACCESS_PROXLITECHIP_HPP

#include "logicalaccess/cards/chip.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CHIP_PROXLITE	"ProxLite"

    /**
     * \brief The Prox Lite base chip class.
     */
    class LIBLOGICALACCESS_API ProxLiteChip : public Chip
    {
    public:

        /**
         * \brief Constructor.
         */
        ProxLiteChip();

        /**
         * \brief Destructor.
         */
        virtual ~ProxLiteChip();

        /**
         * \brief Get the generic card type.
         * \return The generic card type.
         */
	    std::string getGenericCardType() const override { return CHIP_PROXLITE; }

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
	    std::shared_ptr<LocationNode> getRootLocationNode() override;
    };
}

#endif