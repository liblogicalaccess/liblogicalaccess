/**
 * \file mifareultralightcacsacrcommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - ACS ACR.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP

#include "mifareultralightcpcsccommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCACSACR "MifareUltralightCACSACR"
    /**
     * \brief The Mifare Ultralight C commands class for ACS ACR reader.
     */
    class LIBLOGICALACCESS_API MifareUltralightCACSACRCommands : public MifareUltralightCPCSCCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightCACSACRCommands();

		MifareUltralightCACSACRCommands(std::string);

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCACSACRCommands();

    protected:

        virtual std::vector<unsigned char> sendGenericCommand(const std::vector<unsigned char>& data);
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCACSACRCOMMANDS_HPP */