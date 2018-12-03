/**
* \file omnikeyxx22readerunit.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief Omnikey XX22 Reader unit.
*/

#ifndef LOGICALACCESS_OMNIKEYXX22READERUNIT_HPP
#define LOGICALACCESS_OMNIKEYXX22READERUNIT_HPP

#include <logicalaccess/plugins/readers/pcsc/readers/omnikeyxx21readerunit.hpp>

namespace logicalaccess
{
/**
* \brief The Omnikey XX22 reader unit class.
*/
class LLA_READERS_PCSC_API OmnikeyXX22ReaderUnit : public OmnikeyXX21ReaderUnit
{
  public:
    /**
    * \brief Constructor.
    */
    explicit OmnikeyXX22ReaderUnit(const std::string &name);

    /**
    * \brief Destructor.
    */
    virtual ~OmnikeyXX22ReaderUnit();


    /**
    * \brief Get the PC/SC reader unit type.
    * \return The PC/SC reader unit type.
    */
    PCSCReaderUnitType getPCSCType() const override;
};
}

#endif