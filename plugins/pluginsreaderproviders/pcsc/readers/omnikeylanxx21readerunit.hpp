/**
 * \file omnikeyxx21readerunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Omnikey XX21 Reader unit.
 */

#ifndef LOGICALACCESS_OMNIKEYLANXX21READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYLANXX21READERUNIT_HPP

#include "omnikeyxx21readerunit.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Omnikey XX21 reader unit class.
     */
    class LIBLOGICALACCESS_API OmnikeyLANXX21ReaderUnit : public OmnikeyXX21ReaderUnit
    {
    public:

        /**
         * \brief Constructor.
         */
        OmnikeyLANXX21ReaderUnit(const std::string& name);

        /**
         * \brief Destructor.
         */
        virtual ~OmnikeyLANXX21ReaderUnit();

        /**
         * \brief Get the PC/SC reader unit type.
         * \return The PC/SC reader unit type.
         */
        virtual PCSCReaderUnitType getPCSCType() const;

        /**
         * \brief Connect to the reader. Implicit connection on first command sent.
         * \return True if the connection successed.
         */
        virtual bool connectToReader();

        /**
         * \brief Disconnect from reader.
         */
        virtual void disconnectFromReader();

    protected:

        void sendControlMode(DWORD dwControlCode);
    };
}

#endif