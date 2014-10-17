/**
 * \file mifareplus4kchip.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus 4k chip.
 */

#ifndef LOGICALACCESS_MIFAREPLUS4KCHIP_HPP
#define LOGICALACCESS_MIFAREPLUS4KCHIP_HPP

#include "mifarepluschip.hpp"

namespace logicalaccess
{
    /**
     * \brief The 4k MifarePlus base chip class.
     */
    class LIBLOGICALACCESS_API MifarePlus4KChip : public MifarePlusChip
    {
    public:

        /**
         * \brief Constructor
         */
        MifarePlus4KChip();

        /**
         * \brief Destructor.
         */
        virtual ~MifarePlus4KChip();

        /**
         * \brief Get the root location node.
         * \return The root location node.
         */
        virtual std::shared_ptr<LocationNode> getRootLocationNode();

    protected:
    };
}

#endif /* LOGICALACCESS_MIFARE4KCHIP_HPP */