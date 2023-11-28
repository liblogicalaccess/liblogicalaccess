/**
 * \file location.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Location.
 */

#ifndef LOGICALACCESS_LOCATION_HPP
#define LOGICALACCESS_LOCATION_HPP

#include <string>
#include <logicalaccess/xmlserializable.hpp>

namespace logicalaccess
{
/**
 * \brief A location informations.
 */
class LLA_CORE_API Location : public XmlSerializable
{
  public:
    virtual ~Location() = default;

    /**
     * \brief Get the card type for this location.
     * \return The card type.
     */
    virtual std::string getCardType() = 0;

    /**
     * Increment the offset of the location.
     *
     * Todo: Possibly a refactor would be better than this virtual method.
     */
    virtual void incrementOffset(int increment) {
        throw std::runtime_error("not implemented");
    }

    /**
     * \brief Equality operator
     * \param location Location to compare.
     * \return True if equals, false otherwise.
     */
    virtual bool operator==(const Location &location) const;

    /**
     * \brief Inequality operator
     * \param location Location to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const Location &location) const
    {
        return !operator==(location);
    }
};
}

#endif /* LOGICALACCESS_LOCATION_HPP */