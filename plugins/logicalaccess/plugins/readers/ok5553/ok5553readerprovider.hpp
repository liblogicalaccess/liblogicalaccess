/**
 * \file ok5553readerprovider.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief OK5553 card reader provider.
 */

#ifndef LOGICALACCESS_READEROK5553_PROVIDER_HPP
#define LOGICALACCESS_READEROK5553_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/ok5553/ok5553readerunit.hpp>
#include <logicalaccess/plugins/readers/ok5553/lla_readers_ok5553_api.hpp>
#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
#define READER_OK5553 "OK5553"

/**
 * \brief OK5553 Reader Provider class.
 */
class LLA_READERS_OK5553_API OK5553ReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the OK5553ReaderProvider instance.
     */
    static std::shared_ptr<OK5553ReaderProvider> getSingletonInstance();

    /**
     * \brief Get a new OK5553ReaderProvider instance.
     */
    static std::shared_ptr<OK5553ReaderProvider> createInstance();

    /**
     * \brief Destructor.
     */
    ~OK5553ReaderProvider();

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
        return READER_OK5553;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "OK5553";
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
     * param ip The reader address
     * param port The reader port
     * \return A reader unit.
     */
    std::shared_ptr<ReaderUnit> createReaderUnit() override;

  protected:
    /**
     * \brief Constructor.
     */
    OK5553ReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READEROK5553_PROVIDER_HPP */