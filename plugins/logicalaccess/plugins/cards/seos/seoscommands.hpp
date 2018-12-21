/**
 * \file seoscommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SEOS commands.
 */

#ifndef LOGICALACCESS_SEOSCOMMANDS_HPP
#define LOGICALACCESS_SEOSCOMMANDS_HPP

#include <logicalaccess/plugins/cards/seos/lla_cards_seos_api.hpp>
#include <logicalaccess/cards/commands.hpp>
#include <vector>
#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>

namespace logicalaccess
{
#define CMD_SEOS "SEOS"

class SeosChip;

/**
 * \brief The SEOS commands class.
 */
class LLA_CARDS_SEOS_API SeosCommands : public Commands
{
  public:
    /**
     * \brief Constructor.
     */
    SeosCommands()
        : Commands(CMD_SEOS)
    {
    }

    /**
     * \brief Constructor.
     * \param ct The cmd type.
     */
    explicit SeosCommands(std::string cmdtype)
        : Commands(cmdtype)
    {
    }

    virtual ~SeosCommands();

	virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const
    {
        return std::shared_ptr<ISO7816Commands>();
    }

  protected:
    std::shared_ptr<SeosChip> getSeosChip() const;
};
}

#endif