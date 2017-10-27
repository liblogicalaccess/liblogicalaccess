/**
 * \file deisterreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Deister card reader provider.
 */

#ifndef LOGICALACCESS_READERDEISTER_PROVIDER_HPP
#define LOGICALACCESS_READERDEISTER_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "deisterreaderunit.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
#define READER_DEISTER "Deister"

/**
 * \brief Deister Reader Provider class.
 */
class LIBLOGICALACCESS_API DeisterReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the DeisterReaderProvider instance. Singleton because we can only have
     * one COM communication, and Deister can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<DeisterReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~DeisterReaderProvider();

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
        return READER_DEISTER;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Deister";
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
    DeisterReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERDEISTER_PROVIDER_HPP */