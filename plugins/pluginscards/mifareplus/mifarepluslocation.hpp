/**
 * \file mifarepluslocation.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlusLocation.
 */

#ifndef LOGICALACCESS_MIFAREPLUSLOCATION_HPP
#define LOGICALACCESS_MIFAREPLUSLOCATION_HPP

#include "../mifare/mifarelocation.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
/**
 * \brief A  MifarePlus location informations.
 */
class LIBLOGICALACCESS_API MifarePlusLocation : public MifareLocation
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