/**
 * \file mifareplussl3commands.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL3 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP

#include <logicalaccess/cards/aes128key.hpp>
#include "../mifare/mifareaccessinfo.hpp"
#include "../mifare/mifarecommands.hpp"
#include "MifarePlusSL3Auth.hpp"

namespace logicalaccess
{
#define CMD_MIFAREPLUSSL3 	"MifarePlusSL3"

    class LIBLOGICALACCESS_API MifarePlusSL3Commands_NEW : public Commands
    {
    public:
		MifarePlusSL3Commands_NEW() : Commands(CMD_MIFAREPLUSSL3) {}

        explicit MifarePlusSL3Commands_NEW(std::string ct) : Commands(ct) {}

        bool authenticate(int sector, std::shared_ptr<AES128Key> key,
                          MifareKeyType type);

        void resetAuth() const;

        virtual ByteVector readBinaryPlain(unsigned char blockno, size_t len);

    private:
        std::unique_ptr<MifarePlusSL3Auth> auth_;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSL3COMMANDS_HPP*/