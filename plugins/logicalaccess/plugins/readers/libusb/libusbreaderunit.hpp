/**
 * \file libusbreaderunit.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief LibUSB Reader unit.
 */

#ifndef LOGICALACCESS_LIBUSBREADERUNIT_HPP
#define LOGICALACCESS_LIBUSBREADERUNIT_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunitconfiguration.hpp>
#include <logicalaccess/plugins/readers/libusb/lla_readers_libusb_api.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>

#include <libusb.h>
#include <map>

namespace logicalaccess
{
#define READER_LIBUSB "LibUSB"
#define LIBUSB_DEVICE_YUBIKEY_VENDORID 0x1050
#define LIBUSB_DEVICE_ONLYKEY_VENDORID 0x1D50
#define HID_SET_REPORT 0x09

class LibUSBReaderCardAdapter;
class LibUSBReaderProvider;

/**
 * \brief The LibUSB reader unit class.
 */
class LLA_READERS_LIBUSB_API LibUSBReaderUnit : public ReaderUnit
{
  public:
    /**
     * \brief Constructor.
     */
    LibUSBReaderUnit(struct libusb_device* device);

    /**
     * \brief Destructor.
     */
    virtual ~LibUSBReaderUnit();

    /**
     * \brief Get the reader unit name.
     * \return The reader unit name.
     */
    std::string getName() const override;
    
    void setName(const std::string& name);

    /**
     * \brief Get the connected reader unit name.
     * \return The connected reader unit name.
     */
    std::string getConnectedName() override;

    /**
     * \brief Set the card type.
     * \param cardType The card type.
     */
    void setCardType(std::string cardType) override;

    /**
     * \brief Wait for a card insertion.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was inserted, false otherwise. If a card was inserted, the
     * name of the reader on which the insertion was detected is accessible with
     * getReader().
     * \warning If the card is already connected, then the method always fail.
     */
    bool waitInsertion(unsigned int maxwait) override;

    /**
     * \brief Wait for a card removal.
     * \param maxwait The maximum time to wait for, in milliseconds. If maxwait is zero,
     * then the call never times out.
     * \return True if a card was removed, false otherwise. If a card was removed, the
     * name of the reader on which the removal was detected is accessible with
     * getReader().
     */
    bool waitRemoval(unsigned int maxwait) override;

    /**
     * \brief Create the chip object from card type.
     * \param type The card type.
     * \return The chip.
     */
    std::shared_ptr<Chip> createChip(std::string type) override;

    std::vector<unsigned char> getNumber(std::shared_ptr<Chip> chip) override;

    /**
             * \brief Get the first and/or most accurate chip found.
             * \return The single chip.
             */
    std::shared_ptr<Chip> getSingleChip() override;

    /**
     * \brief Get chip available in the RFID rang.
     * \return The chip list.
     */
    std::vector<std::shared_ptr<Chip>> getChipList() override;

    /**
     * \brief Connect to the card.
     * \return True if the card was connected without error, false otherwise.
     *
     * If the card handle was already connected, connect() first call disconnect(). If you
     * intend to do a reconnection, call reconnect() instead.
     */
    bool connect() override;

    /**
     * \brief Disconnect from the reader.
     * \see connect
     *
     * Calling this method on a disconnected reader has no effect.
     */
    void disconnect() override;

    /**
     * \brief Check if the card is connected.
     * \return True if the card is connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Connect to the reader. Implicit connection on first command sent.
     * \return True if the connection successed.
     */
    bool connectToReader() override;

    /**
     * \brief Disconnect from reader.
     */
    void disconnectFromReader() override;

    /**
     * \brief Get a string hexadecimal representation of the reader serial number
     * \return The reader serial number or an empty string on error.
     */
    std::string getReaderSerialNumber() override;

    /**
    * \brief Get the default LibUSB reader/card adapter.
    * \return The default LibUSB reader/card adapter.
    */
    virtual std::shared_ptr<LibUSBReaderCardAdapter> getDefaultLibUSBReaderCardAdapter();

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the LibUSB reader unit configuration.
     * \return The LibUSB reader unit configuration.
     */
    std::shared_ptr<LibUSBReaderUnitConfiguration> getLibUSBConfiguration()
    {
        return std::dynamic_pointer_cast<LibUSBReaderUnitConfiguration>(getConfiguration());
    }

    /**
     * \brief Get the LibUSB reader provider.
     * \return The LibUSB reader provider.
     */
    std::shared_ptr<LibUSBReaderProvider> getLibUSBReaderProvider() const;

    /**
    * \brief Get the LibUSB device.
    * \return The LibUSB device.
    */
    struct libusb_device *getDevice() const
    {
        return d_device;
    }
    
    struct libusb_device_handle *getHandle() const
    {
        return d_device_handle;
    }
    
    void setVendorID(uint16_t vendor_id);

  protected:
    /**
     * \brief The LibUSB device.
     */
    struct libusb_device *d_device;
    
    struct libusb_device *d_tmp_device;
    
    struct libusb_device_handle* d_device_handle;
    
    uint16_t d_vendor_id;
    
    uint16_t d_product_id;
    
    std::string d_serial_number;
    
    std::string d_product;
    
    std::string d_name;
    
    libusb_device* look_for_device(uint16_t vendor_id, uint16_t product_id);

  private:
    /**
     * Call a libusb function and throw an exception if the return code is less than zero.
     */
    template <typename TargetFunction, typename... Args>
    int libusb_safe_call(TargetFunction fct, Args... arg)
    {
        int ret = fct(arg...);
        if (ret < 0)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "LibUSB error: " + std::string(libusb_strerror(d_device)));
        }
        return ret;
    }
};
}

#endif
