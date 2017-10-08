/**
 * \file commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Commands.
 */

#ifndef LOGICALACCESS_COMMANDS_HPP
#define LOGICALACCESS_COMMANDS_HPP

#include "logicalaccess/cards/icommands.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"

namespace logicalaccess
{
    class Chip;

    /**
     * \brief The base commands class for all card commands.
     */
    class LIBLOGICALACCESS_API Commands
    {
    public:
	    virtual ~Commands() = default;

	    Commands() {}

        /**
         * \brief Get the chip.
         * \return The chip.
         */
        virtual std::shared_ptr<Chip> getChip() const { return d_chip.lock(); }

        /**
         * \brief Set the chip.
         * \param chip The chip.
         */
        virtual void setChip(std::shared_ptr<Chip> chip) { d_chip = chip; }

        /**
         * \brief Get the reader/card adapter.
         * \return The reader/card adapter.
         */
        virtual std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; }

        /**
         * \brief Set the reader/card adapter.
         * \param adapter The reader/card adapter.
         */
        virtual void setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; }

    protected:

        /**
         * \brief The reader/card adapter.
         */
        std::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;

        /**
         * \brief The chip.
         */
        std::weak_ptr<Chip> d_chip;
    };
}

#endif