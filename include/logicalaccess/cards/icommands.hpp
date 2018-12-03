/**
 * \file icommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ICommands.
 */

#ifndef LOGICALACCESS_ICOMMANDS_HPP
#define LOGICALACCESS_ICOMMANDS_HPP

#include <logicalaccess/cards/readercardadapter.hpp>

namespace logicalaccess
{
class Chip;

/**
 * \brief The base abstract commands class for all card commands.
 */
class LLA_CORE_API ICommands
{
  public:
    virtual ~ICommands() = default;

    /**
 * \brief Get the chip.
 * \return The chip.
 */
    virtual std::shared_ptr<Chip> getChip() const = 0;

    /**
     * \brief Get the reader/card adapter.
     * \return The reader/card adapter.
     */
    virtual std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const = 0;
};
}

#endif