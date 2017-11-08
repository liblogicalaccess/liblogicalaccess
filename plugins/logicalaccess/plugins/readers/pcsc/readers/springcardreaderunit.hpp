/**
 * \file springcardreaderunit.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard Reader unit.
 */

#ifndef LOGICALACCESS_SPRINGCARDREADERUNIT_HPP
#define LOGICALACCESS_SPRINGCARDREADERUNIT_HPP

#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief The SpringCard reader unit class.
 */
class LIBLOGICALACCESS_API SpringCardReaderUnit : public PCSCReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    explicit SpringCardReaderUnit(const std::string &name);

    /**
     * \brief Destructor.
     */
    virtual ~SpringCardReaderUnit();

    /**
     * \brief Get the PC/SC reader unit type.
     * \return The PC/SC reader unit type.
     */
    PCSCReaderUnitType getPCSCType() const override;

  protected:
    std::shared_ptr<CardProbe> createCardProbe() override;

    std::shared_ptr<ResultChecker> createDefaultResultChecker() const override;
};
}

#endif