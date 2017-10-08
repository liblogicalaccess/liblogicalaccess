#ifndef LIBLOGICALACCESS_MIFARE_CL1356_COMMANDS_HPP
#define LIBLOGICALACCESS_MIFARE_CL1356_COMMANDS_HPP

#include "mifarepcsccommands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	class LIBLOGICALACCESS_API MifareCL1356Commands : public MifarePCSCCommands
    {
    public:
		virtual ~MifareCL1356Commands() = default;

		void increment(uint8_t blockno, uint32_t value) override;

		void decrement(uint8_t blockno, uint32_t value) override;

    };

}

#endif //LIBLOGICALACCESS_MIFARE_ACR1222L_COMMANDS_HPP
