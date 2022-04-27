/**
 * \file yubikeyiso7816commands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Yubikey ISO7816 commands.
 */

#ifndef LOGICALACCESS_YUBIKEYISO7816COMMANDS_HPP
#define LOGICALACCESS_YUBIKEYISO7816COMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/yubikey/yubikeycommands.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>

namespace logicalaccess
{
#define CMD_YUBIKEYISO7816 "YubikeyISO7816"

#define YUBIKEY_INS_PUT 0x01
#define YUBIKEY_INS_DELETE 0x02
#define YUBIKEY_INS_SET_CODE 0x03
#define YUBIKEY_INS_RESET 0x04
#define YUBIKEY_INS_LIST 0xA1
#define YUBIKEY_INS_CALCULATE 0xA2
#define YUBIKEY_INS_VALIDATE 0xA3
#define YUBIKEY_INS_CALCULATE_ALL 0xA4
#define YUBIKEY_INS_SEND_REMAINING 0xA5

/**
 * \brief The Yubikey ISO7816 commands base class.
 * Based on https://docs.yubico.com/yesdk/users-manual/application-oath/oath-commands.html / https://developers.yubico.com/OATH/YKOATH_Protocol.html for OATH
 * and https://docs.yubico.com/yesdk/users-manual/application-otp/otp-commands.html for OTP.
 * For FIPS-201 PIV features, please use FIPS201Commands instead.
 */
class LLA_READERS_ISO7816_API YubikeyISO7816Commands : public YubikeyCommands
{
  public:
    /**
     * \brief Constructor.
     */
    YubikeyISO7816Commands();

    explicit YubikeyISO7816Commands(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~YubikeyISO7816Commands();

    /**
     * \brief Select the Yubikey OATH application.
	 * \return A challenge is returned if the authentication object is set. In that case an authentication is required for all commands except VALIDATE and RESET.
     */
    YubikeySelectResponse selectYubikeyOATH() override;
    
    /**
     * \brief Select the old Yubikey OTP application.
	 * \return A challenge is returned.
     */
    ByteVector selectYubikeyOTP() override;
    
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
    ByteVector otp_getResponse(YubikeySlot slot, const ByteVector& challenge) override;
    
    /**
     * \brief Reads the serial number of the YubiKey if it is allowed by the configuration. Note that certain keys, such as the Security Key by Yubico, do not have serial numbers.
     * \return The Yubikey serial number.
     */
    ByteVector otp_getSerialNumber() override;
    
    /**
     * \brief Reads configuration and metadata information about the YubiKey. Similar commands exist in other applications. The Command APDU may be different, however the data in the Response APDU will be of identical format.
     * \return The device information.
     */
    std::vector<TLVPtr> otp_getDeviceInfo() override;
    
    /**
     * \brief Determines whether or not the device is loaded with FIPS capable firmware, as well as if the key is currently in a FIPS compliant state.
     * \return True if FIPS compliant, false otherwise.
     * \remarks Just because a key may be branded FIPS or have FIPS capable firmware loaded, does not mean that the YubiKey is FIPS compliant. Configurations on the key need to be locked or otherwise protected in order to claim compliant behavior.
     */
    bool otp_queryFIPSMode() override;
    
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
    void oath_put(const ByteVector& name, YubikeyHMAC algorithm, uint8_t digits, const ByteVector& key, YubikeyProperty property = PROP_NONE, const ByteVector& imf = ByteVector()) override;

    /**
     * \brief Deletes an existing credential.
     * \param name The credential name to delete.
     */
    void oath_delete(const ByteVector& name) override;

    /**
     * \brief Configures Authentication. If length 0 is sent, authentication is removed.
	 * The key to be set is expected to be a user-supplied UTF-8 encoded password passed through 1000 rounds of PBKDF2 with the ID from select used as salt. 16 bytes of that are used.
	 * When configuring authentication you are required to send an 8 byte challenge and one authentication-response with that key, in order to confirm that the application and the host software can calculate the same response for that key.
	 * \param algorithm The HMAC algorithm to use.
	 * \param key The HMAC key.
     * \param challenge The challenge.
     */
    void oath_setCode(YubikeyHMAC algorithm, const ByteVector& key, const ByteVector& challenge) override;
	
	/**
     * \brief Lists configured credentials.
     * \return The credentials lists.
     */
    std::vector<YubikeyListItem> oath_list() override;
	
	/**
     * \brief Resets the application to just-installed state.
     */
    void oath_reset() override;
	
	/**
     * \brief Performs CALCULATE for one named credential.
     * \param name The credential name.
     * \param challenge The challenge.
     * \param truncate Truncate the response, full response otherwise.
     * \return The calculate response.
     */
    YubikeyCalculateResponse oath_calculate(const ByteVector& name, const ByteVector& challenge, bool truncate = false) override;
	
	/**
     * \brief Validates authentication (mutually). The challenge for this comes from the SELECT command.
	 * The response if computed by performing the correct HMAC function of that challenge with the correct key.
	 * A new challenge is then sent to the application, together with the response.
	 * The application will then respond with a similar calculation that the host software can verify.
     * \param challenge The challenge.
	 * \param challenge The host calculated response.
     * \return The calculate response.
     */
    ByteVector oath_validate(const ByteVector& challenge, const ByteVector& response) override;
	
	/**
     * \brief Performs CALCULATE for all available credentials, returns name + response for TOTP and just name for HOTP and credentials requiring touch.
     * \param challenge The challenge.
     * \param truncate Truncate the response, full response otherwise.
     * \return The calculate responses.
     */
    std::vector<YubikeyCalculateResponse> oath_calculateAll(const ByteVector& challenge, bool truncate) override;
	
	/**
     * \brief Gets remaining data if everything didn’t fit in previous response (response code was 61xx).
     * \return Continued data where previous command left off.
     */
    ByteVector oath_sendRemainingInstruction() override;
    
    std::shared_ptr<ISO7816Commands> getISO7816Commands() const override
    {
        auto command = std::make_shared<ISO7816ISO7816Commands>();
        command->setChip(getChip());
        command->setReaderCardAdapter(getReaderCardAdapter());
        return command;
    }

	/**
     * \brief Get the ISO7816 reader/card adapter.
     * \return The ISO7816 reader/card adapter.
     */
    std::shared_ptr<ISO7816ReaderCardAdapter> getISO7816ReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<ISO7816ReaderCardAdapter>(
            getReaderCardAdapter());
    }
};
}

#endif