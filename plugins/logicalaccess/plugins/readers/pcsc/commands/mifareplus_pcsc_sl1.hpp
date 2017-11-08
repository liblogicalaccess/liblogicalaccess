//
// Created by xaqq on 6/25/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUS_PSCS_SL1_H
#define LIBLOGICALACCESS_MIFAREPLUS_PSCS_SL1_H


#include <string>
#include <vector>
#include <iostream>

#include <logicalaccess/plugins/cards/mifareplus/mifareplussl1commands.hpp>
#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>

namespace logicalaccess
{
using MifarePlusSL1PCSCCommands =
    MifarePlusSL1Policy<MifarePlusSL1Commands, MifarePCSCCommands>;
}


#endif // LIBLOGICALACCESS_MIFAREPLUS_PSCS_SL1_H
