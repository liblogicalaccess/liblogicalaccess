/**
 * \file elatecreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Elatec card reader provider.
 */

#ifndef LOGICALACCESS_READERELATEC_PROVIDER_HPP
#define LOGICALACCESS_READERELATEC_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/elatec/elatecreaderunit.hpp>

#include <string>
#include <vector>

namespace logicalaccess
{
#define READER_ELATEC "Elatec"

/**
 * \brief Elatec Reader Provider class.
 */
class LLA_READERS_ELATEC_API ElatecReaderProvider : public ReaderProvider
{
  public:
    /**
     * \brief Get the ElatecReaderProvider instance. Singleton because we can only have
     * one COM communication, and Elatec can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<ElatecReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~ElatecReaderProvider();

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
        return READER_ELATEC;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Elatec";
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
    ElatecReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERELATEC_PROVIDER_H */