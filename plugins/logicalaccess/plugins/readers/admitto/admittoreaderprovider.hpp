/**
 * \file admittoreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto card reader provider.
 */

#ifndef LOGICALACCESS_READERADMITTO_PROVIDER_HPP
#define LOGICALACCESS_READERADMITTO_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/admitto/admittoreaderunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
#define READER_ADMITTO "Admitto"

/**
 * \brief Admitto Reader Provider class.
 */
class LIBLOGICALACCESS_API AdmittoReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the AdmittoReaderProvider instance. Singleton because we can only have
     * one COM communication, and Admitto can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<AdmittoReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~AdmittoReaderProvider();

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
        return READER_ADMITTO;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Phg Admitto";
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
    AdmittoReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERADMITTO_PROVIDER_H */