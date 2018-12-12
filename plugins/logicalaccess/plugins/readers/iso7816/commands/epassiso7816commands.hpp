/**
 * \file epassiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief EPass ISO7816 commands.
 */

#ifndef LOGICALACCESS_EPASSISO7816COMMANDS_HPP
#define LOGICALACCESS_EPASSISO7816COMMANDS_HPP

#include <logicalaccess/plugins/cards/epass/epasscommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

namespace logicalaccess
{
#define CMD_EPASSISO7816 "EPassISO7816"
class LLA_READERS_ISO7816_API EPassISO7816Commands : public EPassCommands
{
  public:
    EPassISO7816Commands();

	explicit EPassISO7816Commands(std::string ct);

	virtual ~EPassISO7816Commands();

	std::shared_ptr<ISO7816Commands> getISO7816Commands() const override
    {
        auto command = std::make_shared<ISO7816ISO7816Commands>();
        command->setChip(getChip());
        command->setReaderCardAdapter(getReaderCardAdapter());
        return command;
    }
};
} // namespace logicalaccess
#endif