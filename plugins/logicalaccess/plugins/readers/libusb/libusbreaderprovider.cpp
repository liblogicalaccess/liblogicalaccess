/**
 * \file libusbreaderprovider.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LibUSB reader provider.
 */

#include <algorithm>
#include <logicalaccess/plugins/readers/libusb/libusbreaderprovider.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
LibUSBReaderProvider::LibUSBReaderProvider()
    : ReaderProvider()
{
    int r = libusb_init(&d_context);
    if (r < 0 || d_context == nullptr)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Unable to init libusb");
}

std::shared_ptr<LibUSBReaderProvider> LibUSBReaderProvider::createInstance()
{
    std::shared_ptr<LibUSBReaderProvider> instance =
        std::shared_ptr<LibUSBReaderProvider>(new LibUSBReaderProvider());
    instance->refreshReaderList();

    return instance;
}

LibUSBReaderProvider::~LibUSBReaderProvider()
{
    LibUSBReaderProvider::release();
}

void LibUSBReaderProvider::release()
{
    if (d_context != nullptr)
    {
        libusb_exit(d_context);
        d_context = nullptr;
    }
}

std::shared_ptr<ReaderUnit> LibUSBReaderProvider::createReaderUnit()
{
    LOG(LogLevel::INFOS) << "Creating default LibUSB reader unit. "
                         << "This will need further configuration to be used.";

    auto unit = std::make_shared<LibUSBReaderUnit>(nullptr);
    unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
    return unit;
}

bool LibUSBReaderProvider::refreshReaderList()
{
    libusb_device **devices;
    ssize_t device_count = libusb_get_device_list(d_context, &devices);
    
    std::shared_ptr<LibUSBReaderUnit> anyYubikeyReader(new LibUSBReaderUnit(nullptr));
    anyYubikeyReader->setVendorID(LIBUSB_DEVICE_YUBIKEY_VENDORID);
    anyYubikeyReader->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
    anyYubikeyReader->setName("Any Yubikey");
    d_readers.push_back(anyYubikeyReader);
    std::shared_ptr<LibUSBReaderUnit> anyOnlykeyReader(new LibUSBReaderUnit(nullptr));
    anyYubikeyReader->setVendorID(LIBUSB_DEVICE_ONLYKEY_VENDORID);
    anyOnlykeyReader->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
    anyOnlykeyReader->setName("Any Onlykey");
    d_readers.push_back(anyOnlykeyReader);

    LOG(DEBUGS) << "Found " << device_count << " devices.";
    for (ssize_t i = 0; i < device_count && devices[i] != nullptr; ++i)
    {
        struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(devices[i], &desc);
		if (r >= 0)
        {
            // We may want to create dedicated reader unit implementation if additional devices with different logics get implemented later on
            if (desc.idVendor == LIBUSB_DEVICE_YUBIKEY_VENDORID || desc.idVendor == LIBUSB_DEVICE_ONLYKEY_VENDORID)
            {
                LOG(DEBUGS) << "Found matching device with Vendor ID " << desc.idVendor << " and Product ID " << desc.idProduct;
                auto unit = std::make_shared<LibUSBReaderUnit>(devices[i]);
                unit->setReaderProvider(std::weak_ptr<ReaderProvider>(shared_from_this()));
                d_readers.push_back(unit);
            }
        }
        else
        {
            LOG(ERRORS) << "Failed to get device descriptor for device #" << i;
        }
    }
    libusb_free_device_list(devices, 1);
    
    return true;
}
}