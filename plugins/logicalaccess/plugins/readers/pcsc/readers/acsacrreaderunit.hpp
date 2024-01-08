/**
 * \file acsacrreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ACS ACR Reader unit.
 */

#ifndef LOGICALACCESS_ACSACRREADERUNIT_HPP
#define LOGICALACCESS_ACSACRREADERUNIT_HPP

#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief The ACS ACR reader unit class.
 */
class LLA_READERS_PCSC_API ACSACRReaderUnit : public PCSCReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    explicit ACSACRReaderUnit(const std::string &name);

    /**
     * \brief Destructor.
     */
    virtual ~ACSACRReaderUnit();

    /**
     * \brief Get the PC/SC reader unit type.
     * \return The PC/SC reader unit type.
     */
    PCSCReaderUnitType getPCSCType() const override;

    /**
    * \brief Get the reader firmware version.
    * \return The firmware version.
    */
    std::string getFirmwareVersion();

  protected:
    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;
};
}

#endif