/**
 * \file axesstmc13readerprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC13 card reader provider.
 */

#ifndef LOGICALACCESS_READERAXESSTMC13_PROVIDER_H
#define LOGICALACCESS_READERAXESSTMC13_PROVIDER_H

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "axesstmc13readerunit.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
#define READER_AXESSTMC13 "AxessTMC13"

/**
 * \brief AxessTMC13 Reader Provider class.
 */
class LIBLOGICALACCESS_API AxessTMC13ReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the AxessTMC13ReaderProvider instance. Singleton because we can only
     * have one COM communication, and AxessTMC13 can't have shared access, so we share
     * the same reader unit.
     */
    static std::shared_ptr<AxessTMC13ReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~AxessTMC13ReaderProvider();

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
        return READER_AXESSTMC13;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Axess TMC 13Mhz";
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
    AxessTMC13ReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERAXESSTMC13_PROVIDER_H */