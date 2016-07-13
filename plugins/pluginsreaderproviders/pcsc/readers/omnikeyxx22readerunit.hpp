/**
* \file omnikeyxx22readerunit.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Omnikey XX22 Reader unit.
*/

#ifndef LOGICALACCESS_OMNIKEYXX22READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX22READERUNIT_HPP

#include "omnikeyxx21readerunit.hpp"

namespace logicalaccess
{
    /**
    * \brief The Omnikey XX22 reader unit class.
    */
    class LIBLOGICALACCESS_API OmnikeyXX22ReaderUnit : public OmnikeyXX21ReaderUnit
    {
    public:
        /**
        * \brief Constructor.
        */
        OmnikeyXX22ReaderUnit(const std::string& name);

        /**
        * \brief Destructor.
        */
        virtual ~OmnikeyXX22ReaderUnit();


        /**
        * \brief Get the PC/SC reader unit type.
        * \return The PC/SC reader unit type.
        */
        virtual PCSCReaderUnitType getPCSCType() const;
    };
}

#endif