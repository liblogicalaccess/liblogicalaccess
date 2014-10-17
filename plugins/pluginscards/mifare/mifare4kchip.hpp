/**
 * \file mifare4kchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare 4k chip.
 */

#ifndef LOGICALACCESS_MIFARE4KCHIP_HPP
#define LOGICALACCESS_MIFARE4KCHIP_HPP

#include "mifarechip.hpp"

namespace logicalaccess
{
#define CHIP_MIFARE4K	"Mifare4K"

    /**
     * \brief The 4k Mifare base chip class.
     */
    class LIBLOGICALACCESS_API Mifare4KChip : public MifareChip
    {
    public:

        /**
         * \brief Constructor
         */
        Mifare4KChip();

        /**
         * \brief Destructor.
         */
        virtual ~Mifare4KChip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

    protected:
    };
}

#endif /* LOGICALACCESS_MIFARE4KCHIP_HPP */