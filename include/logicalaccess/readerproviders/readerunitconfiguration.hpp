/**
 * \file readerunitconfiguration.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Reader unit configuration.
 */

#ifndef LOGICALACCESS_READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_READERUNITCONFIGURATION_HPP

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/xmlserializable.hpp>

namespace logicalaccess
{
/**
 * \brief The reader unit configuration base class. Describe the configuration for a
 * reader unit.
 */
class LLA_CORE_API ReaderUnitConfiguration : public XmlSerializable
{
  public:
    /**
     * \brief Constructor.
     */
    explicit ReaderUnitConfiguration(std::string rpt);

    /**
     * \brief Destructor.
     */
    virtual ~ReaderUnitConfiguration();

    /**
     * \brief Get the reader provider type.
     * \return The reader provider type.
     */
    std::string getRPType() const;

    /**
     * \brief Reset the configuration to the default one.
     */
    virtual void resetConfiguration() = 0;

  protected:
    /**
     * \brief The associated reader provider type for the reader unit configuration.
     */
    std::string d_readerProviderType;
};
}

#endif