/**
* \file omnikeyxx23readerunit.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Omnikey XX23 Reader unit.
*/

#ifndef LOGICALACCESS_OMNIKEYXX23READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX23READERUNIT_HPP

#include "omnikeyxx21readerunit.hpp"

namespace logicalaccess
{
/**
* \brief The Omnikey XX23 reader unit class.
*/
class LIBLOGICALACCESS_API OmnikeyXX23ReaderUnit : public OmnikeyXX21ReaderUnit
{
  public:
    /**
    * \brief Constructor.
    */
    explicit OmnikeyXX23ReaderUnit(const std::string &name);

    /**
    * \brief Destructor.
    */
    virtual ~OmnikeyXX23ReaderUnit();


    /**
    * \brief Get the PC/SC reader unit type.
    * \return The PC/SC reader unit type.
    */
    PCSCReaderUnitType getPCSCType() const override;
};
}

#endif