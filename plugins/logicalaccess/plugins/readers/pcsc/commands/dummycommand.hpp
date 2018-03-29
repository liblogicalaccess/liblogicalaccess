//
// Created by xaqq on 4/28/15.
//

#pragma once

#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{
#define CMD_DUMMY "Dummy"
/**
 * This is a dummy implementation of commands.
 *
 * It exists so that we know explicitly that we are working with
 * dummy commands rather than using the base Commands class.
 *
 * A DummyCommand only goal is to make the reader unit object available
 * to the chip through the reader card adapter and data transport,
 * even if it makes no use of them.
 *
 * As of now, DummyCommands are used by Prox and SEOS cards.
 */
class DummyCommands : public Commands
{
  public:
    DummyCommands()
        : Commands(CMD_DUMMY)
    {
    }
};
}
