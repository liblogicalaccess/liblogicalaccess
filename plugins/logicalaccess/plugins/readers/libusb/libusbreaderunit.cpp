/**
 * \file libusbreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief LibUSB reader unit.
 */

#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderprovider.hpp>
#include <logicalaccess/plugins/readers/libusb/readercardadapters/libusbreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/libusb/yubikeydatatransport.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeychip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/yubikeyiso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>

#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/date_time.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <logicalaccess/cards/chip.hpp>

namespace logicalaccess
{

LibUSBReaderUnit::LibUSBReaderUnit(struct libusb_device* device)
    : ReaderUnit(READER_LIBUSB)
    , d_device(device)
    , d_device_handle(nullptr)
    , d_tmp_device(nullptr)
    , d_vendor_id(0)
    , d_product_id(0)
{
    d_readerUnitConfig.reset(new LibUSBReaderUnitConfiguration());
    ReaderUnit::setDefaultReaderCardAdapter(std::make_shared<LibUSBReaderCardAdapter>());

    std::shared_ptr<LibUSBDataTransport> dataTransport(new LibUSBDataTransport());
    ReaderUnit::setDataTransport(dataTransport);
    d_card_type = "UNKNOWN";
    
    if (device != nullptr)
    {
        libusb_ref_device(device);
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(device, &desc);
        if (r >= 0)
        {
            d_vendor_id = desc.idVendor;
            d_product_id = desc.idProduct;
        }
    }
}

LibUSBReaderUnit::~LibUSBReaderUnit()
{
    if (d_device != nullptr)
    {
        libusb_unref_device(d_device);
        d_device = nullptr;
    }
    LibUSBReaderUnit::disconnectFromReader();
}

std::string LibUSBReaderUnit::getName() const
{
    return d_name;
}

void LibUSBReaderUnit::setName(const std::string& name)
{
    d_name = name;
}

std::string LibUSBReaderUnit::getConnectedName()
{
    if (!d_product.empty())
    {
        return d_product + "(#" +  + ")";
    }
    
    return d_name;
}

void LibUSBReaderUnit::setCardType(std::string cardType)
{
    d_card_type = cardType;
}

struct libusb_device* LibUSBReaderUnit::look_for_device(uint16_t vendor_id, uint16_t product_id)
{
    struct libusb_device *mdevice = nullptr;
    struct libusb_device **devices;
    ssize_t device_count = libusb_get_device_list(getLibUSBReaderProvider()->getContext(), &devices);
    if (device_count > 0)
    {
        for (size_t i = 0; i < device_count && devices[i] != nullptr && mdevice == nullptr; ++i)
        {
            struct libusb_device_descriptor desc;
            int r = libusb_get_device_descriptor(devices[i], &desc);
            if (r >= 0)
            {
                if ((vendor_id <= 0 || desc.idVendor == vendor_id) || (product_id <= 0 || desc.idProduct == product_id))
                {
                    mdevice = devices[i];
                    libusb_ref_device(mdevice);
                }
            }
        }
    }
    libusb_free_device_list(devices, 1);
    return mdevice;
}

bool LibUSBReaderUnit::waitInsertion(unsigned int maxwait)
{
    if (Settings::getInstance()->SeeWaitInsertionLog)
    {
        LOG(LogLevel::INFOS) << "Waiting card insertion...";
    }

    if (d_device == nullptr)
    {
        d_tmp_device = nullptr;
        boost::posix_time::ptime currentDate =
            boost::posix_time::second_clock::local_time();
        boost::posix_time::ptime maxDate =
            currentDate + boost::posix_time::milliseconds(maxwait);

        while (d_tmp_device == nullptr && currentDate < maxDate)
        {
            struct libusb_device *device = look_for_device(d_vendor_id, d_product_id);
            if (d_tmp_device != nullptr)
            {
                LOG(DEBUGS) << "Found matching device with Vendor ID " << d_vendor_id << " and Product ID " << d_product_id;
                d_tmp_device = device;
            }
            else
            {
                currentDate = boost::posix_time::second_clock::local_time();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
    else
    {
        libusb_ref_device(d_device);
        d_tmp_device = d_device;
    }
    
    if (d_tmp_device != nullptr)
    {
        d_insertedChip = createChip("Yubikey");
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "No underlying libusb reader associated with this object.");
    }
    return (d_insertedChip != nullptr);
}

bool LibUSBReaderUnit::waitRemoval(unsigned int maxwait)
{
    LOG(DEBUGS) << "Waiting for card removal.";
    std::chrono::steady_clock::time_point wait_until(std::chrono::steady_clock::now() +
                                                     std::chrono::milliseconds(maxwait));
    bool removed = false;

    if (d_tmp_device != nullptr)
    {
        // If we can open the device, it means it is still plugged
        while (!removed && (std::chrono::steady_clock::now() < wait_until || maxwait == 0))
        {
            struct libusb_device_handle* device_handle;
            int r = libusb_open(d_tmp_device, &device_handle);
            if (r == 0 && device_handle != nullptr)
            {
                libusb_close(device_handle);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            else
            {
                removed = true;
            }
        }
    }
    else
    {
        removed = true;
    }
    
    if (removed)
    {
        libusb_unref_device(d_tmp_device);
        d_tmp_device = nullptr;
        d_insertedChip = nullptr;
    }

    return removed;
}

bool LibUSBReaderUnit::connect()
{
    LOG(INFOS) << "Attempting to connect to LibUSB device \"" << d_name << "\"";
    
    if (d_tmp_device != nullptr)
    {
        int r = libusb_open(d_tmp_device, &d_device_handle);
        if (r == 0)
        {
            r = libusb_kernel_driver_active(d_device_handle, 0);
            if (r == 1)
            {
                r = libusb_detach_kernel_driver(d_device_handle, 0);
                if (r != 0)
                {
                    LOG(LogLevel::WARNINGS) << "Cannot detach kernel driver from device.";
                }
            }
        }
        else
        {
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "Cannot open underlying libusb device.");
        }
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "No underlying libusb reader associated with this object.");
    }
    
    if (d_device_handle != nullptr)
    {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(d_tmp_device, &desc);
        if (r >= 0)
        {
            char usbstring[256];
            int r = libusb_get_string_descriptor_ascii(d_device_handle, desc.iProduct, reinterpret_cast<unsigned char *>(usbstring), sizeof(usbstring));
            if (r > 0)
            {
                d_product = std::string(usbstring);
            }
            r = libusb_get_string_descriptor_ascii(d_device_handle, desc.iSerialNumber, reinterpret_cast<unsigned char *>(usbstring), sizeof(usbstring));
            if (r > 0)
            {
                d_serial_number = std::string(usbstring);
                d_insertedChip->setChipIdentifier(BufferHelper::fromHexString(d_serial_number));
            }
        }
    }
    else
    {
        LOG(ERRORS) << "Failed to open LibUSB device.";
    }
    return (d_device_handle != nullptr);
}

void LibUSBReaderUnit::disconnect()
{
    if (d_device_handle != nullptr)
    {
        libusb_attach_kernel_driver(d_device_handle, 0);
        libusb_close(d_device_handle);
        d_device_handle = nullptr;
    }
}

std::shared_ptr<Chip> LibUSBReaderUnit::createChip(std::string type)
{
    LOG(LogLevel::INFOS) << "Create chip " << type;
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);

    if (chip)
    {
        LOG(LogLevel::INFOS) << "Chip (" << chip->getCardType()
                             << ") created, creating other associated objects...";

        std::shared_ptr<ReaderCardAdapter> rca = getDefaultReaderCardAdapter();
        std::shared_ptr<Commands> commands;
        std::shared_ptr<DataTransport> dt;
        std::shared_ptr<ResultChecker> resultChecker(
            new ISO7816ResultChecker()); // default one

        if (type == "Yubikey")
        {
            commands.reset(new YubikeyISO7816Commands());
            dt = std::make_shared<YubikeyDataTransport>();
        }

        if (rca)
        {
            rca->setResultChecker(resultChecker);
            if (dt)
            {
                LOG(LogLevel::INFOS) << "Data transport forced to a specific one.";
                rca->setDataTransport(dt);
            }

            dt = rca->getDataTransport();
            if (dt)
            {
                dt->setReaderUnit(shared_from_this());
            }
        }

        if (commands)
        {
            commands->setReaderCardAdapter(rca);
            commands->setChip(chip);
            chip->setCommands(commands);
        }

        LOG(LogLevel::INFOS) << "Object creation done.";
    }

    return chip;
}

std::shared_ptr<Chip> LibUSBReaderUnit::getSingleChip()
{
    return d_insertedChip;
}

std::vector<std::shared_ptr<Chip>> LibUSBReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> v;
    v.push_back(d_insertedChip);
    return v;
}

std::string LibUSBReaderUnit::getReaderSerialNumber()
{
    return d_serial_number;
}

std::shared_ptr<LibUSBReaderCardAdapter> LibUSBReaderUnit::getDefaultLibUSBReaderCardAdapter()
{
    return std::dynamic_pointer_cast<LibUSBReaderCardAdapter>(getDefaultReaderCardAdapter());
}

bool LibUSBReaderUnit::isConnected()
{
    // Reader = Chip here
    return (d_device_handle != nullptr);
}

bool LibUSBReaderUnit::connectToReader()
{
    return true;
}

void LibUSBReaderUnit::disconnectFromReader()
{
}

void LibUSBReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    ReaderUnit::serialize(node);
    node.put("Name", d_name);
    node.put("VendorID", d_vendor_id);
    node.put("ProductID", d_product_id);
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void LibUSBReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    d_name = node.get_child("Name").get_value<std::string>();
    d_vendor_id = node.get_child("VendorID").get_value<uint16_t>();
    d_product_id = node.get_child("ProductID").get_value<uint16_t>();
    ReaderUnit::unSerialize(node);
}

std::shared_ptr<LibUSBReaderProvider> LibUSBReaderUnit::getLibUSBReaderProvider() const
{
    return std::dynamic_pointer_cast<LibUSBReaderProvider>(getReaderProvider());
}

std::vector<unsigned char> LibUSBReaderUnit::getNumber(std::shared_ptr<Chip> chip)
{
    return ReaderUnit::getNumber(chip);
}

void LibUSBReaderUnit::setVendorID(uint16_t vendor_id)
{
    d_vendor_id = vendor_id;
}
}
