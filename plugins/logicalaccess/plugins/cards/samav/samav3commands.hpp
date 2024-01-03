/**
 * \file SAMAV3Commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAMAV3Commands commands.
 */

#ifndef LOGICALACCESS_SAMAV3COMMANDS_HPP
#define LOGICALACCESS_SAMAV3COMMANDS_HPP

#include <logicalaccess/plugins/cards/samav/samcommands.hpp>

namespace logicalaccess
{
template <typename T, typename S>
class SAMAV3Commands : public ICommands
{
  public:
    virtual ByteVector encipherKeyEntry(unsigned char keyno,
                                  unsigned char targetKeyno,
                                  unsigned short changeCounter,
                                  unsigned char channel = 0,
                                  const ByteVector& targetSamUid = ByteVector(),
                                  const ByteVector& divInput = ByteVector()) = 0;
};
}

#endif /* LOGICALACCESS_SAMAV3COMMANDS_HPP */