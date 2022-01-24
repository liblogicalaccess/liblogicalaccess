/**
 * \file stidstrreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR card reader provider.
 */

#ifndef LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP
#define LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP

#include <logicalaccess/plugins/readers/iso7816/iso7816readerprovider.hpp>
#include <logicalaccess/plugins/readers/stidstr/lla_readers_stidstr_api.hpp>

namespace logicalaccess
{
#define READER_STIDSTR "STidSTR"

/**
 * \brief STidSTR Reader Provider class.
 */
class LLA_READERS_STIDSTR_API STidSTRReaderProvider : public ISO7816ReaderProvider
{
  public:
    /**
     * \brief Get the STidSTReaderProvider instance. Singleton because we can only have
     * one COM communication, and Deister can't have shared access, so we share the same
     * reader unit.
     */
    static std::shared_ptr<STidSTRReaderProvider> getSingletonInstance();

    /**
     * \brief Destructor.
     */
    ~STidSTRReaderProvider();

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
        return READER_STIDSTR;
    }

    /**
     * \brief Get the reader provider name.
     * \return The reader provider name.
     */
    std::string getRPName() const override
    {
        return "STid STR";
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
    STidSTRReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;
};
}

#endif /* LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP */