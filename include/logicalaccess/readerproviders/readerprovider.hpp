/**
 * \file readerprovider.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Reader provider.
 */

#ifndef LOGICALACCESS_READER_PROVIDER_HPP
#define LOGICALACCESS_READER_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <map>

namespace logicalaccess
{
/**
 * \brief A reader unit list.
 */
typedef std::vector<std::shared_ptr<ReaderUnit>> ReaderList;

bool hasEnding(std::string const &fullString, std::string ending);

/**
 * \brief A Reader Provider base class. It provide a available list of specific reader
 * kind.
 */
class LLA_CORE_API ReaderProvider
    : public std::enable_shared_from_this<ReaderProvider>
{
  public:
    /**
     * \brief Constructor.
     */
    ReaderProvider();

    /**
     * \brief Destructor.
     */
    virtual ~ReaderProvider();

    /**
     * \brief Release the provider resources.
     */
    virtual void release() = 0;

    /**
     * \brief List all readers of the system.
     * \return True if the list was updated, false otherwise.
     */
    virtual bool refreshReaderList() = 0;

    /**
     * \brief Get reader list for this reader provider.
     * \return The reader list.
     */
    virtual const ReaderList &getReaderList() = 0;

    /**
     * \brief Lock until one or all ready are detected.
     * \return The reader list with one or all the ReaderUnit.
     */
    virtual const ReaderList waitForReaders(std::vector<std::string> readers,
                                            double maxwait, bool all);

    /**
     * \brief Get the reader provider type.
     * \return The reader provider type.
     */
    virtual std::string getRPType() const = 0;

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    virtual std::string getRPName() const = 0;

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    virtual std::shared_ptr<ReaderUnit> createReaderUnit() = 0;

    /**
     * \brief Get the reader provider object from the reader provider type.
     * \param rpt The reader provider type.
     * \return The reader provider object.
     */
    static std::shared_ptr<ReaderProvider> getReaderProviderFromRPType(std::string rpt);
};
}

#endif /* LOGICALACCESS_READER_PROVIDER_HPP */