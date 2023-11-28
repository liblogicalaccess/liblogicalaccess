/**
 * \file location.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Base class location.
 */

#include <logicalaccess/cards/location.hpp>
#include <time.h>
#include <stdlib.h>

namespace logicalaccess
{
bool Location::operator==(const Location &location) const
{
    return (typeid(location) == typeid(*this));
}
}