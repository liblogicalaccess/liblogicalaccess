//
// Created by xaqq on 4/28/15.
//

#pragma once

#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{
    /**
     * Prox card doesn't support any command.
     * This is a dummy command class. Its goal is to make
     * the reader unit object available to the chip through the reader card adapter
     * and data transport, even if it makes no use of them.
     */
    class ProxCommands : public Commands
    {

    };
}
