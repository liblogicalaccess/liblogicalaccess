/**
 * \file cherryreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Cherry Reader unit.
 */

#ifndef LOGICALACCESS_CHERRYREADERUNIT_HPP
#define LOGICALACCESS_CHERRYREADERUNIT_HPP

#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief The Cherry reader unit class.
 */
class LLA_READERS_PCSC_API CherryReaderUnit : public PCSCReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    explicit CherryReaderUnit(const std::string &name);

    /**
     * \brief Destructor.
     */
    virtual ~CherryReaderUnit();

    /**
     * \brief Get the PC/SC reader unit type.
     * \return The PC/SC reader unit type.
     */
    PCSCReaderUnitType getPCSCType() const override;
};
}

#endif