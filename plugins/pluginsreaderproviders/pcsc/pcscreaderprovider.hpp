/**
 * \file pcscreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC card reader provider.
 */

#ifndef LOGICALACCESS_READERPCSC_PROVIDER_HPP
#define LOGICALACCESS_READERPCSC_PROVIDER_HPP

#include "../iso7816/iso7816readerprovider.hpp"
#include "pcscreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
#define READER_PCSC "PCSC"

/**
 * \brief PC/SC Reader Provider class.
 */
class LIBLOGICALACCESS_API PCSCReaderProvider : public ISO7816ReaderProvider
{
  protected:
    /**
     * \brief Constructor.
     */
    PCSCReaderProvider();

  public:
    /**
     * \brief Destructor.
     */
    ~PCSCReaderProvider();

    /**
     * \brief Release the provider resources.
     */
    void release() override;

    /**
     * \brief Create a new PC/SC reader provider instance.
     * \return The PC/SC reader provider instance.
     */
    static std::shared_ptr<PCSCReaderProvider> createInstance();

    /**
     * \brief Get the reader provider type.
     * \return The reader provider type.
     */
    std::string getRPType() const override
    {
        return READER_PCSC;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "PC/SC";
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
        return d_system_readers;
    }

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    std::shared_ptr<ReaderUnit> createReaderUnit() override;

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    std::shared_ptr<ISO7816ReaderUnit>
    createReaderUnit(std::string readerunitname) override;

    /**
     * \brief Get the reader group list.
     * \return The reader group list.
     */
    std::vector<std::string> getReaderGroupList() const;

    /**
     * \brief Get the SCard context.
     * \return The SCard context.
     */
    SCARDCONTEXT getContext() const
    {
        return d_scc;
    }

  protected:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

    /**
     * \brief The system readers list.
     */
    ReaderList d_system_readers;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    /**
     * \brief The context.
     */
    SCARDCONTEXT d_scc;
};
}

#endif /* LOGICALACCESS_READERPCSC_PROVIDER_HPP */