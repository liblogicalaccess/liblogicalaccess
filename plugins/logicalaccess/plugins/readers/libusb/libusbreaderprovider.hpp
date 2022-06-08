/**
 * \file libusbreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LibUSB card reader provider.
 */

#ifndef LOGICALACCESS_READERLIBUSB_PROVIDER_HPP
#define LOGICALACCESS_READERLIBUSB_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <libusb.h>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>


namespace logicalaccess
{
#define READER_LIBUSB "LibUSB"

/**
 * \brief LibUSB Reader Provider class.
 */
class LLA_READERS_LIBUSB_API LibUSBReaderProvider : public ReaderProvider
{
  public:
    /**
    * \brief Create a new LibUSB reader provider instance.
    * \return The LibUSB reader provider instance.
    */
    static std::shared_ptr<LibUSBReaderProvider> createInstance();

    /**
     * \brief Destructor.
     */
    ~LibUSBReaderProvider();

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
        return READER_LIBUSB;
    }

    /**
 * \brief Get the reader provider name.
 * \return The reader provider name.
 */
    std::string getRPName() const override
    {
        return "LibUSB";
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
    * \brief Get the LibUSB context.
    * \return The LibUSB context.
    */
    libusb_context *getContext() const
    {
        return d_context;
    }

  protected:
    /**
     * \brief Constructor.
     */
    LibUSBReaderProvider();

    /**
     * \brief The reader list.
     */
    ReaderList d_readers;

    /**
     * \brief The LibUSB Context.
    */
    libusb_context *d_context;
};
}

#endif /* LOGICALACCESS_READERLIBUSB_PROVIDER_HPP */
