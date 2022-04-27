/**
 * \file yubikeydatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey data transport.
 * \remarks Based on https://github.com/Yubico/yubikey-personalization
 */

#include <logicalaccess/plugins/readers/libusb/yubikeydatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/readers/libusb/libusbreaderunit.hpp>
#include <libusb.h>

namespace logicalaccess
{
YubikeyDataTransport::YubikeyDataTransport()
    : LibUSBDataTransport()
{
}

YubikeyDataTransport::~YubikeyDataTransport()
{
}

void YubikeyDataTransport::send(const std::vector<unsigned char> &data)
{
    EXCEPTION_ASSERT_WITH_LOG(getLibUSBReaderUnit(), LibLogicalAccessException,
                              "The LibUSB reader unit object is null. We cannot send.");

    if (data.size() > 0)
    {
        LOG(LogLevel::COMS) << "APDU command: " << BufferHelper::getHex(data);
        YK_FRAME frame;
        unsigned char repbuf[FEATURE_RPT_SIZE];
        int i, seq;
        int ret = 0;
        unsigned char *ptr, *end;

        EXCEPTION_ASSERT_WITH_LOG(data.size() <= sizeof(frame.payload), LibLogicalAccessException,
                              "Wrong size.");

        /* Insert data and set slot # */

        memset(&frame, 0, sizeof(frame));
        memcpy(frame.payload, &data[0], data.size());
        frame.slot = getLibUSBReaderUnit()->getLibUSBConfiguration()->getSlot();

        /* Append slot checksum */

        i = yk_crc16 (std::vector<unsigned char>(frame.payload, frame.payload + sizeof(frame.payload)));
        frame.crc = yk_endian_swap_16(i);

        /* Chop up the data into parts that fits into the payload of a
           feature report. Set the sequence number | 0x80 in the end
           of the feature report. When the Yubikey has processed it,
           it will clear this byte, signaling that the next part can be
           sent */

        ptr = (unsigned char *) &frame;
        end = (unsigned char *) &frame + sizeof(frame);

        for (seq = 0; ptr < end; seq++) {
            int all_zeros = 1;
            /* Ignore parts that are all zeroes except first and last
               to speed up the transfer */

            for (i = 0; i < (FEATURE_RPT_SIZE - 1); i++) {
                if ((repbuf[i] = *ptr++)) all_zeros = 0;
            }
            if (all_zeros && (seq > 0) && (ptr < end))
                continue;

            /* sequence number goes into lower bits of last byte */
            repbuf[i] = seq | SLOT_WRITE_FLAG;

            /* When the Yubikey clears the SLOT_WRITE_FLAG, the
             * next part can be sent.
             */
            yk_wait_for_key_status(frame.slot, WAIT_FOR_WRITE_FLAG, 0, false, SLOT_WRITE_FLAG);
            usb_write(std::vector<unsigned char>(repbuf, repbuf + FEATURE_RPT_SIZE), REPORT_TYPE_FEATURE, 0);
        }
    }
}

std::vector<unsigned char> YubikeyDataTransport::receive(long int timeout)
{
    return receive(0, timeout);
}

std::vector<unsigned char> YubikeyDataTransport::receive(size_t readlen, long int timeout)
{
    EXCEPTION_ASSERT_WITH_LOG(getLibUSBReaderUnit(), LibLogicalAccessException,
                              "The LibUSB reader unit object is null. We cannot receive.");
                              
    std::vector<unsigned char> data;
	std::vector<unsigned char> buf;
	size_t bytes_read = 0;

	/* Wait for the key to turn on RESP_PENDING_FLAG */
	data = yk_wait_for_key_status(getLibUSBReaderUnit()->getLibUSBConfiguration()->getSlot(), 0, static_cast<int>(timeout), true, RESP_PENDING_FLAG);
    if (data.size() < 1)
		return buf;

	/* The first part of the response was read by yk_wait_for_key_status(). We need
	 * to copy it to buf.
	 */
    buf.insert(buf.end(), data.begin(), data.end());
	bytes_read += data.size() - 1;

	while (bytes_read + FEATURE_RPT_SIZE <= readlen || readlen == 0) {
		data = usb_read(REPORT_TYPE_FEATURE, 0, FEATURE_RPT_SIZE, static_cast<int>(timeout));
		if (data[FEATURE_RPT_SIZE - 1] & RESP_PENDING_FLAG) {
			/* The lower five bits of the status byte has the response sequence
			 * number. If that gets reset to zero we are done.
			 */
			if ((data[FEATURE_RPT_SIZE - 1] & 31) == 0) {
				if (readlen > 0) {
					/* Size of response is known. Verify CRC. */
					readlen += 2;
                    buf.resize(readlen);
					int crc = yk_crc16(buf);
                    EXCEPTION_ASSERT_WITH_LOG(crc == YK_CRC_OK_RESIDUAL, LibLogicalAccessException,
                              "Wrong Yubikey checksum.");

					/* since we get data in chunks of 7 we need to round expect bytes out to the closest higher multiple of 7 */
					if(readlen % 7 != 0) {
						readlen += 7 - (readlen % 7);
					}

                    EXCEPTION_ASSERT_WITH_LOG(bytes_read == readlen, LibLogicalAccessException,
                              "Wrong size.");
				}

				/* Reset read mode of Yubikey before returning. */
				yk_force_key_update();

				return buf;
			}

			buf.insert(buf.end(), data.begin(), data.end());
			bytes_read += data.size() - 1;
		} else {
			/* Reset read mode of Yubikey before returning. */
			yk_force_key_update();

			return std::vector<unsigned char>();
		}
	}

	/* We're out of buffer space, abort reading */
	yk_force_key_update();

	THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Yubikey receive timeout.");
}

void YubikeyDataTransport::yk_force_key_update()
{
	std::vector<unsigned char> buf;
    buf.resize(FEATURE_RPT_SIZE - 1, 0x00);
	buf.push_back(DUMMY_REPORT_WRITE); /* Invalid sequence = update only */
	usb_write(buf, REPORT_TYPE_FEATURE, 0);
}

/* Wait for the Yubikey to either set or clear (controlled by the boolean logic_and)
 * the bits in mask.
 *
 * The slot parameter is here for future purposes only.
 */
std::vector<unsigned char> YubikeyDataTransport::yk_wait_for_key_status(uint8_t slot, unsigned int flags,
			   unsigned int max_time_ms,
			   bool logic_and, unsigned char mask)
{
	std::vector<unsigned char> data;

	unsigned int sleepval = 1;
	unsigned int slept_time = 0;
	int blocking = 0;

	/* Non-zero slot breaks on Windows (libusb-1.0.8-win32), while working fine
	 * on Linux (and probably MacOS X).
	 *
	 * The YubiKey doesn't support per-slot status anyways at the moment (2.2),
	 * so we just set it to 0 (meaning slot 1).
	 */
	slot = 0;

	while (slept_time < max_time_ms) {
		Sleep(sleepval);
		slept_time += sleepval;
		/* exponential backoff, up to 500 ms */
		sleepval *= 2;
		if (sleepval > 500)
			sleepval = 500;

		/* Read a status report from the key */
		data = usb_read(REPORT_TYPE_FEATURE, slot, FEATURE_RPT_SIZE);
        if (data.size() < 1)
			return data;

		/* The status byte from the key is now in last byte of data */
		if (logic_and) {
			/* Check if Yubikey has SET the bit(s) in mask */
			if ((data[FEATURE_RPT_SIZE - 1] & mask) == mask) {
				return data;
			}
		} else {
			/* Check if Yubikey has CLEARED the bit(s) in mask */
			if (! (data[FEATURE_RPT_SIZE - 1] & mask)) {
				return data;
			}
		}

		/* Check if Yubikey says it will wait for user interaction */
		if ((data[FEATURE_RPT_SIZE - 1] & RESP_TIMEOUT_WAIT_FLAG) == RESP_TIMEOUT_WAIT_FLAG) {
			if ((flags & YK_FLAG_MAYBLOCK) == YK_FLAG_MAYBLOCK) {
				if (! blocking) {
					/* Extend timeout first time we see RESP_TIMEOUT_WAIT_FLAG. */
					blocking = 1;
					max_time_ms += 256 * 1000;
				}
			} else {
				/* Reset read mode of Yubikey before aborting. */
				yk_force_key_update();
                THROW_EXCEPTION_WITH_LOG(
                    LibLogicalAccessException,
                    "Yubikey would block but this is not expected.");
			}
		} else {
			if (blocking) {
				/* YubiKey timed out waiting for user interaction */
				break;
			}
		}
	}

	THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Yubikey wait timeout.");
}

uint16_t YubikeyDataTransport::yk_crc16(const std::vector<unsigned char>& data) const
{
    uint16_t m_crc = 0xffff;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        int i, j;
        m_crc ^= (uint8_t) *it & 0xFF;
        for (i = 0; i < 8; i++)
        {
            j = m_crc & 1;
            m_crc >>= 1;
            if (j)
                m_crc ^= 0x8408;
        }
    }

    return m_crc;
}

uint16_t YubikeyDataTransport::yk_endian_swap_16(uint16_t x) const
{
	static int testflag = -1;

	if (testflag == -1) {
		uint16_t testword = 0x0102;
		unsigned char *testchars = (unsigned char *)&testword;
		if (*testchars == '\1')
			testflag = 1; /* Big endian arch, swap needed */
		else
			testflag = 0; /* Little endian arch, no swap needed */
	}

	if (testflag)
		x = (x >> 8) | ((x & 0xff) << 8);

	return x;
}
}
