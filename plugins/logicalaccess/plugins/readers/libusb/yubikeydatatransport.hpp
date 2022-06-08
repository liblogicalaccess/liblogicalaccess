/**
 * \file yubikeydatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey transport for reader/card commands.
 */

#ifndef LOGICALACCESS_YUBIKEYDATATRANSPORT_HPP
#define LOGICALACCESS_YUBIKEYDATATRANSPORT_HPP

#include <logicalaccess/plugins/readers/libusb/libusbdatatransport.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <list>

namespace logicalaccess
{
#define RESP_TIMEOUT_WAIT_MASK 0x1f
#define RESP_TIMEOUT_WAIT_FLAG 0x20
#define RESP_PENDING_FLAG 0x40
#define SLOT_WRITE_FLAG 0x80
#define DUMMY_REPORT_WRITE 0x8f
#define YK_FLAG_MAYBLOCK 0x01 << 16
#define YK_CRC_OK_RESIDUAL 0xf0b8
#define	SLOT_DATA_SIZE 64
#define WAIT_FOR_WRITE_FLAG 1150

typedef struct
{
  unsigned char payload[SLOT_DATA_SIZE];	// Frame payload
  unsigned char slot;		// Slot # field
  unsigned short crc;		// CRC field
  unsigned char filler[3];	// Filler
} YK_FRAME;

/**
 * \brief An Yubikey data transport class.
 */
class LLA_READERS_LIBUSB_API YubikeyDataTransport : public LibUSBDataTransport
{
  public:
    /**
     * \brief Constructor.
     */
    YubikeyDataTransport();

    /**
     * \brief Destructor.
     */
    virtual ~YubikeyDataTransport();

    /**
     * \brief Send the data using rpleth protocol computation.
     * \param data The data to send.
     */
    void send(const std::vector<unsigned char> &data) override;

    /**
     * \brief Receive data from reader.
     * \param timeout The time to wait data.
     * \return The data from reader.
     */
    std::vector<unsigned char> receive(long int timeout = 5000) override;
    
    std::vector<unsigned char> receive(size_t readlen, long int timeout);
    
    void yk_force_key_update();
    
    std::vector<unsigned char> yk_wait_for_key_status(uint8_t slot, unsigned int flags,
			   unsigned int max_time_ms,
			   bool logic_and, unsigned char mask);
               
    uint16_t yk_crc16(const std::vector<unsigned char>& data) const;
    
    uint16_t yk_endian_swap_16(uint16_t x) const;
};
}

#endif /* LOGICALACCESS_YUBIKEYDATATRANSPORT_HPP */