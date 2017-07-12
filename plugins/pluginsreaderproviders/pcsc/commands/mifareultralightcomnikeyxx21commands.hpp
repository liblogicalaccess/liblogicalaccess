/**
 * \file mifareultralightcomnikeyxx21commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - Omnikey xx21.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX21COMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX21COMMANDS_HPP

#include "mifareultralightcpcsccommands.hpp"

namespace logicalaccess
{
#define CMD_MIFAREULTRALIGHTCOMNIKEYXX21 "MifareUltralightCOmnikeyXX21"
    /**
     * \brief The Mifare Ultralight C commands class for Omnikey xx21 reader.
     */
    class LIBLOGICALACCESS_API MifareUltralightCOmnikeyXX21Commands : public MifareUltralightCPCSCCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifareUltralightCOmnikeyXX21Commands();

		MifareUltralightCOmnikeyXX21Commands(std::string);

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCOmnikeyXX21Commands();

    protected:

		virtual void startGenericSession();

		virtual void stopGenericSession();

        virtual std::vector<unsigned char> sendGenericCommand(const std::vector<unsigned char>& data);
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX21COMMANDS_HPP */