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

        /**
         * \brief Destructor.
         */
        virtual ~MifareUltralightCOmnikeyXX21Commands();

    protected:

	    void startGenericSession() override;

	    void stopGenericSession() override;

	    ByteVector sendGenericCommand(const ByteVector& data) override;
    };
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCOMNIKEYXX21COMMANDS_HPP */