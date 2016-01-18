/**
 * \file acsacrreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ACS ACR Reader unit.
 */

#ifndef LOGICALACCESS_ACSACRREADERUNIT_HPP
#define LOGICALACCESS_ACSACRREADERUNIT_HPP

#include "../pcscreaderunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The ACS ACR reader unit class.
     */
    class LIBLOGICALACCESS_API ACSACRReaderUnit : public PCSCReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
        ACSACRReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~ACSACRReaderUnit();

        /**
         * \brief Get the PC/SC reader unit type.
         * \return The PC/SC reader unit type.
         */
        virtual PCSCReaderUnitType getPCSCType() const;

        /**
        * \brief Get the reader firmware version.
        * \return The firmware version.
        */
        std::string getFirmwareVersion();

    protected:
        virtual std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;
    };
}

#endif