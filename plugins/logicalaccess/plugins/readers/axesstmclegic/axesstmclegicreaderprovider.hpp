/**
 * \file axesstmclegicreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic card reader provider.
 */

#ifndef LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_HPP
#define LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/axesstmclegic/axesstmclegicreaderunit.hpp>
#include <logicalaccess/plugins/readers/axesstmclegic/lla_readers_axesstmclegic_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
#define READER_AXESSTMCLEGIC "AxessTMCLegic"

/**
 * \brief AxessTMCLegic Reader Provider class.
 */
class LLA_READERS_AXESSTMCLEGIC_API AxessTMCLegicReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the AxessTMCLegicReaderProvider instance. Singleton because we can only
     * have one COM communication, and AxessTMCLegic can't have shared access, so we share
     * the same reader unit.
     */
    static std::shared_ptr<AxessTMCLegicReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~AxessTMCLegicReaderProvider();

    /**
     * \brief Release the provider resources.
     */
    void release() override;

    /**
     * \brief Get the reader provider type.
     * \return The reader provider type.
     */
    std::string getRPType() const override
    {
        return READER_AXESSTMCLEGIC;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "AxessTMC Legic";
    }

    /**
     * \brief List all readers of the system.
     * \return True if the list was updated, false otherwise.
     */
    bool refreshReaderList() override;

    /**
     * \brief Get reader list for this reader provider.
     * \return The reader list.
     */
    const ReaderList &getReaderList() override
    {
        return d_readers;
    }

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    std::shared_ptr<ReaderUnit> createReaderUnit() override;

  protected:
    /**
     * \brief Constructor.
     */
    AxessTMCLegicReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERAXESSTMCLEGIC_PROVIDER_HPP */