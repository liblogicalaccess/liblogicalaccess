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

class SEOSChip;

/**
 * \brief The SEOS commands class.
 */
class LLA_CARDS_SEOS_API SEOSCommands : public Commands
{
  public:
    /**
     * \brief Constructor.
     */
    SEOSCommands()
        : Commands(CMD_SEOS)
    {
    }

    /**
     * \brief Constructor.
     * \param ct The cmd type.
     */
    explicit SEOSCommands(std::string cmdtype)
        : Commands(cmdtype)
    {
    }

    virtual ~SEOSCommands();

	virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const
    {
        return std::shared_ptr<ISO7816Commands>();
    }

  protected:
    std::shared_ptr<SEOSChip> getSEOSChip() const;
};
}

#endif