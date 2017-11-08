/**
 * \file gunneboreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Gunnebo card reader provider.
 */

#ifndef LOGICALACCESS_READERGUNNEBO_PROVIDER_HPP
#define LOGICALACCESS_READERGUNNEBO_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/gunnebo/gunneboreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
#define READER_GUNNEBO "Gunnebo"

/**
 * \brief Gunnebo Reader Provider class.
 */
class LIBLOGICALACCESS_API GunneboReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the GunneboReaderProvider instance. Singleton because we can only have
     * one COM communication, and Gunnebo can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<GunneboReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~GunneboReaderProvider();

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
        return READER_GUNNEBO;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Gunnebo CRS1306AA1";
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
    GunneboReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERGUNNEBO_PROVIDER_HPP */