/**
 * \file gigatmsreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief GIGA-TMS card reader provider.
 */

#ifndef LOGICALACCESS_READERGIGATMS_PROVIDER_HPP
#define LOGICALACCESS_READERGIGATMS_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "gigatmsreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
#define READER_GIGATMS "GigaTMS"

/**
 * \brief GIGA-TMS Reader Provider class.
 */
class LIBLOGICALACCESS_API GigaTMSReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the GigaTMSReaderProvider instance. Singleton because we can only have
     * one COM communication, and GIGA-TMS can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<GigaTMSReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~GigaTMSReaderProvider();

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
        return READER_GIGATMS;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "GigaTMS";
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
    GigaTMSReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERGIGATMS_PROVIDER_HPP */