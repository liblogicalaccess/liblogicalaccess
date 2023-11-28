/**
 * \file commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Commands.
 */

#ifndef LOGICALACCESS_COMMANDS_HPP
#define LOGICALACCESS_COMMANDS_HPP

#include <logicalaccess/cards/icommands.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>

namespace logicalaccess
{
class Chip;

/**
 * \brief The base commands class for all card commands.
 */
class LLA_CORE_API Commands : public ICommands
{
  public:
    /**
     * \brief Constructor.
     */
    Commands() = delete;

    virtual ~Commands();

    /**
     * \brief Get the chip.
     * \return The chip.
     */
    std::shared_ptr<Chip> getChip() const override
    {
        return d_chip.lock();
    }

    /**
     * \brief Set the chip.
     * \param chip The chip.
     */
    virtual void setChip(std::shared_ptr<Chip> chip)
    {
        d_chip = chip;
    }

    /**
     * \brief Get the reader/card adapter.
     * \return The reader/card adapter.
     */
    std::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const override
    {
        return d_readerCardAdapter;
    }

    /**
     * \brief Set the reader/card adapter.
     * \param adapter The reader/card adapter.
     */
    virtual void setReaderCardAdapter(std::shared_ptr<ReaderCardAdapter> adapter)
    {
        d_readerCardAdapter = adapter;
    }

    /**
     * \brief Get the cmd name.
     * \return The cmd name.
     */
    virtual const std::string &getCmdType() const
    {
        return d_commandtype;
    }

    /**
     * \brief Set the cmd name.
     * \param chip The cmd name.
     */
    virtual void setCmdType(const std::string &command_type)
    {
        d_commandtype = command_type;
    }

  protected:
    /**
     * \brief Constructor.
     * \param commandtype The command type name.
     */
    explicit Commands(std::string commandtype)
        : d_commandtype(commandtype)
    {
    }

    /**
     * \brief The reader/card adapter.
     */
    std::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;

    /**
     * \brief The chip.
     */
    std::weak_ptr<Chip> d_chip;

    /**
     * \brief The command type.
     */
    std::string d_commandtype;
};
}

#endif