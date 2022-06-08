/**
 * \file yubikeycommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey commands.
 */

#ifndef LOGICALACCESS_YUBIKEYCOMMANDS_HPP
#define LOGICALACCESS_YUBIKEYCOMMANDS_HPP

#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>
#include <logicalaccess/plugins/cards/yubikey/lla_cards_yubikey_api.hpp>
#include <logicalaccess/tlv.hpp>
#include <vector>

namespace logicalaccess
{
#define CMD_YUBIKEY "Yubikey"

/**
 * \brief The HMAC algorithm
 */
typedef enum
{
    HMAC_UNKNOWN = 0x00,
    HMAC_SHA1    = 0x01,
    HMAC_SHA256  = 0x02,
    HMAC_SHA512  = 0x03,
    HMAC_TYPE_HOTP    = 0x10,
    HMAC_TYPE_TOTP    = 0x20
} YubikeyHMAC;

/**
 * \brief Yubikey properties
 */
typedef enum
{
	PROP_NONE             = 0x00,
    PROP_ONLY_INCREASE    = 0x01, /* Enforces that a challenge is always higher than the previous */
    PROP_REQUIRE_TOUCH    = 0x02 /* Require button press to generate OATH codes */
} YubikeyProperty;

/**
 * \brief Yubikey slot. Only used for the old slot-based Yubikey API
 */
typedef enum
{
	SLOT_DUMMY                   = 0x00,
    SLOT_CONFIG_1                = 0x01,
    SLOT_NAV                     = 0x02,
    SLOT_CONFIG_2                = 0x03,
    SLOT_UPDATE_1                = 0x04,
    SLOT_UPDATE_2                = 0x05,
    SLOT_SWAP                    = 0x06,
    SLOT_NDEF_1                  = 0x08,
    SLOT_NDEF_2                  = 0x09,
    SLOT_DEVICE_SERIAL           = 0x10,
    SLOT_DEVICE_CONFIGURATION    = 0x11,
    SLOT_SCAN_MAP                = 0x12,
    SLOT_YUBIKEY_4_CAPABILITIES  = 0x13,
    SLOT_CHALLENGE_OTP_1         = 0x20,
    SLOT_CHALLENGE_OTP_2         = 0x28,
    SLOT_CHALLENGE_HMAC_1        = 0x30,
    SLOT_CHALLENGE_HMAC_2        = 0x38
} YubikeySlot;

typedef struct s_YubikeySelectResponse
{
	ByteVector version;
	ByteVector name;
	YubikeyHMAC algorithm = HMAC_UNKNOWN;
	ByteVector challenge;
} YubikeySelectResponse;

typedef struct s_YubikeyCalculateResponse
{
	ByteVector name;
	uint8_t digits = 0;
	ByteVector response;
} YubikeyCalculateResponse;

typedef struct s_YubikeyListItem
{
    YubikeyHMAC algorithm = HMAC_UNKNOWN;
	ByteVector name;
} YubikeyListItem;

#define YUBIKEY_TLV_DEVINFO_AVAILABLE_CAPABILITIES_USB  0x01 /* USB Applications and capabilities that are available for use on this YubiKey. */
#define YUBIKEY_TLV_DEVINFO_SERIAL_NUMBER               0x02 /* Returns the serial number of the YubiKey (if present and visible). */
#define YUBIKEY_TLV_DEVINFO_ENABLED_CAPABILITIES_USB    0x03 /* Applications that are currently enabled over USB on this YubiKey. */
#define YUBIKEY_TLV_DEVINFO_FORM_FACTOR                 0x04 /* Specifies the form factor of the YubiKey (USB-A, USB-C, Nano, etc.) */
#define YUBIKEY_TLV_DEVINFO_FIRMWARE_VERSION            0x05 /* The Major.Minor.Patch version number of the firmware running on the YubiKey. */
#define YUBIKEY_TLV_DEVINFO_AUTOEJECT_TIMEOUT           0x06 /* Timeout in (ms?) before the YubiKey automatically "ejects" itself. */
#define YUBIKEY_TLV_DEVINFO_CHALLENGE_RESPONSE_TIMEOUT  0x07 /* The period of time (in seconds) after which the OTP challenge-response command should timeout. */
#define YUBIKEY_TLV_DEVINFO_DEVICE_FLAGS                0x08 /* Device flags that can control device-global behavior. */
#define YUBIKEY_TLV_DEVINFO_CONFIGURATION_LOCK          0x0A /* Indicates whether or not the YubiKey's configuration has been locked by the user. */
#define YUBIKEY_TLV_DEVINFO_AVAILABLE_CAPABILITIES_NFC  0x0D /* NFC Applications and capabilities that are available for use on this YubiKey. */
#define YUBIKEY_TLV_DEVINFO_ENABLED_CAPABILITIES_NFC    0x0E /* Applications that are currently enabled over USB on this YubiKey. */

#define YUBIKEY_TLV_NAME 0x71
#define YUBIKEY_TLV_NAME_LIST 0x72
#define YUBIKEY_TLV_KEY 0x73
#define YUBIKEY_TLV_CHALLENGE 0x74
#define YUBIKEY_TLV_FULL_RESPONSE 0x75
#define YUBIKEY_TLV_TRUNCATED_RESPONSE 0x76
#define YUBIKEY_TLV_HOTP_RESPONSE 0x77
#define YUBIKEY_TLV_PROPERTY 0x78
#define YUBIKEY_TLV_VERSION 0x79
#define YUBIKEY_TLV_IMF 0x7a
#define YUBIKEY_TLV_ALGORITHM 0x7b
#define YUBIKEY_TLV_TOUCH_RESPONSE 0x7c

/**
 * \brief The Yubikey commands class.
 */
class LLA_CARDS_YUBIKEY_API YubikeyCommands : public Commands
{
  public:
  
    /**
    * \brief Constructor.
    */
    YubikeyCommands()
        : Commands(CMD_YUBIKEY)
    {
    }

    /**
    * \brief Constructor.
    * \param ct The cmd type.
    */
    explicit YubikeyCommands(std::string cmdtype)
        : Commands(cmdtype)
    {
    }
  
    /**
     * \brief Select the Yubikey OATH application.
	 * \return A challenge is returned if the authentication object is set. In that case an authentication is required for all commands except VALIDATE and RESET.
     */
    virtual YubikeySelectResponse selectYubikeyOATH() = 0;
    
    /**
     * \brief Select the old Yubikey OTP application.
	 * \return A challenge is returned.
     */
    virtual ByteVector selectYubikeyOTP() = 0;
    
    /******************************************************
     *                                                    *
     *                   === OTP ===                      *
     *                                                    *
     * All OTP management commands are skipped for now as *
     * Yubikey Manager app can be used for initial setup. *
     *                                                    *
     ******************************************************/
    
    /**
     * \brief Challenge/Response operation.
     * \param slot Target slot.
     * \param challenge Challenge to transmit.
     * \return The response.
     */
    virtual ByteVector otp_getResponse(YubikeySlot slot, const ByteVector& challenge) = 0;
    
    /**
     * \brief Reads the serial number of the YubiKey if it is allowed by the configuration. Note that certain keys, such as the Security Key by Yubico, do not have serial numbers.
     * \return The Yubikey serial number.
     */
    virtual ByteVector otp_getSerialNumber() = 0;
    
    /**
     * \brief Reads configuration and metadata information about the YubiKey. Similar commands exist in other applications. The Command APDU may be different, however the data in the Response APDU will be of identical format.
     * \return The device information.
     */
    virtual std::vector<TLVPtr> otp_getDeviceInfo() = 0;
    
    /**
     * \brief Determines whether or not the device is loaded with FIPS capable firmware, as well as if the key is currently in a FIPS compliant state.
     * \return True if FIPS compliant, false otherwise.
     * \remarks Just because a key may be branded FIPS or have FIPS capable firmware loaded, does not mean that the YubiKey is FIPS compliant. Configurations on the key need to be locked or otherwise protected in order to claim compliant behavior.
     */
    virtual bool otp_queryFIPSMode() = 0;
    
    /******************************************************
     *                                                    *
     *                   === OAUTH ==                     *
     *                                                    *
     ******************************************************/

    /**
     * \brief Adds a new (or overwrites) OATH credential.
     * \param name The credential name.
	 * \param algorithm The HMAC algorithm.
	 * \param digits Number of digits in OATH code.
     * \param key The HMAC key.
	 * \param property OATH properties.
	 * \param imf IMF (only valid for HOTP).
     */
    virtual void oath_put(const ByteVector& name, YubikeyHMAC algorithm, uint8_t digits, const ByteVector& key, YubikeyProperty property = PROP_NONE, const ByteVector& imf = ByteVector()) = 0;

    /**
     * \brief Deletes an existing credential.
     * \param name The credential name to delete.
     */
    virtual void oath_delete(const ByteVector& name) = 0;

    /**
     * \brief Configures Authentication. If length 0 is sent, authentication is removed.
	 * The key to be set is expected to be a user-supplied UTF-8 encoded password passed through 1000 rounds of PBKDF2 with the ID from select used as salt. 16 bytes of that are used.
	 * When configuring authentication you are required to send an 8 byte challenge and one authentication-response with that key, in order to confirm that the application and the host software can calculate the same response for that key.
     * \param name The credential name.
	 * \param algorithm The HMAC algorithm to use.
	 * \param key The HMAC key.
     * \param challenge The challenge.
     */
    virtual void oath_setCode(YubikeyHMAC algorithm, const ByteVector& key, const ByteVector& challenge) = 0;
	
	/**
     * \brief Lists configured credentials.
     * \return The credentials lists.
     */
    virtual std::vector<YubikeyListItem> oath_list() = 0;
	
	/**
     * \brief Resets the application to just-installed state.
     */
    virtual void oath_reset() = 0;
	
	/**
     * \brief Performs CALCULATE for one named credential.
     * \param name The credential name.
     * \param challenge The challenge.
     * \param truncate Truncate the response, full response otherwise.
     * \return The calculate response.
     */
    virtual YubikeyCalculateResponse oath_calculate(const ByteVector& name, const ByteVector& challenge, bool truncate = false) = 0;
	
	/**
     * \brief Validates authentication (mutually). The challenge for this comes from the SELECT command.
	 * The response if computed by performing the correct HMAC function of that challenge with the correct key.
	 * A new challenge is then sent to the application, together with the response.
	 * The application will then respond with a similar calculation that the host software can verify.
     * \param challenge The challenge.
	 * \param challenge The host calculated response.
     * \return The calculate response.
     */
    virtual ByteVector oath_validate(const ByteVector& challenge, const ByteVector& response) = 0;
	
	/**
     * \brief Performs CALCULATE for all available credentials, returns name + response for TOTP and just name for HOTP and credentials requiring touch.
     * \param challenge The challenge.
     * \param truncate Truncate the response, full response otherwise.
     * \return The calculate responses.
     */
    virtual std::vector<YubikeyCalculateResponse> oath_calculateAll(const ByteVector& challenge, bool truncate = false) = 0;
	
	/**
     * \brief Gets remaining data if everything didn’t fit in previous response (response code was 61xx).
     * \return Continued data where previous command left off.
     */
    virtual ByteVector oath_sendRemainingInstruction() = 0;
    
    virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const = 0;
};
}

#endif