/**
 * \file mifarepluslocation.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusLocation.
 */

#ifndef LOGICALACCESS_MIFAREPLUSLOCATION_HPP
#define LOGICALACCESS_MIFAREPLUSLOCATION_HPP

#include <logicalaccess/plugins/cards/mifare/mifarelocation.hpp>
#include <logicalaccess/plugins/cards/mifareplus/lla_cards_mifareplus_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief A  MifarePlus location informations.
 */
class LLA_CARDS_MIFAREPLUS_API MifarePlusLocation : public MifareLocation
{
  public:
    /**
     * \brief Constructor.
     */
    MifarePlusLocation();

    /**
     * \brief Destructor.
     */
    virtual ~MifarePlusLocation();
};
}

#endif /* LOGICALACCESS_MIFAREPLUSLOCATION_HPP */