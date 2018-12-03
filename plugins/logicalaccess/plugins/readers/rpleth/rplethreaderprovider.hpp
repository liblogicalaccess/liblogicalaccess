/**
 * \file rplethreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth card reader provider.
 */

#ifndef LOGICALACCESS_READERRPLETH_PROVIDER_HPP
#define LOGICALACCESS_READERRPLETH_PROVIDER_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816readerprovider.hpp>
#include <logicalaccess/plugins/readers/rpleth/lla_readers_rpleth_api.hpp>
#include <string>
#include <vector>

namespace logicalaccess
{
#define READER_RPLETH "Rpleth"

/**
 * \brief Rpleth Reader Provider class.
 */
class LLA_READERS_RPLETH_API RplethReaderProvider : public ISO7816ReaderProvider
{
  public:
    /**
     * \brief Get the RplethReaderProvider instance. Singleton because we can only have
     * one TCP communication, and Rpleth can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<RplethReaderProvider> getSingletonInstance();

    /**
     * \brief Get a new RplethReaderProvider instance.
     */
    static std::shared_ptr<RplethReaderProvider> createInstance();

    /**
     * \brief Destructor.
     */
    ~RplethReaderProvider();

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
        return READER_RPLETH;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "Rpleth";
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

    /**
    * \brief Create a new reader unit for the reader provider.
    * \return A reader unit.
    */
    std::shared_ptr<ISO7816ReaderUnit>
    createReaderUnit(std::string readerunitname) override;

  protected:
    /**
     * \brief Constructor.
     */
    RplethReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERRPLETH_PROVIDER_HPP */