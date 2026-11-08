#pragma once

#include <cstddef>
#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816response.hpp>
#include <logicalaccess/plugins/cards/nedapmobileaccess/lla_cards_nedapmobileaccess_api.hpp>

namespace logicalaccess
{
#define CMD_NEDAP_MOBILE_ACCESS "NedapMobileAccess"

class ISO7816Commands;

enum class NedapMobileAccessCommunicationMode
{
    // Data is clear; AES EV1 still protects the response with a CMAC.
    Plain,
    // Data is clear and the trailing AES-CMAC is verified and removed.
    Mac,
    // Decrypt the AES-CBC response and verify its CRC32 and padding.
    Enciphered
};

class LLA_CARDS_NEDAPMOBILEACCESS_API NedapMobileAccessCommands : public Commands
{
  public:
    NedapMobileAccessCommands();

    virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const = 0;
    virtual ByteVector selectApplication(const ByteVector &dfName) = 0;

    // The key must already be diversified for the selected application.
    void authenticateAES(const ByteVector &diversifiedKey, unsigned char keyNumber = 0);
    // randomData is the 16-byte value read from file 0 in AID 0xF056C0.
    void authenticateAES(const ByteVector &diversificationKey, const ByteVector &uid,
                         unsigned int aid,
                         const ByteVector &randomData, unsigned char keyNumber = 0);
    ISO7816Response readData(unsigned char fileNumber, size_t offset, size_t length,
                             NedapMobileAccessCommunicationMode communicationMode);
    ISO7816Response requestUserAuthentication(const ByteVector &authenticationRequest);
    ISO7816Response endTransaction();

    bool isAuthenticated() const
    {
        return !sessionKey_.empty();
    }

  protected:
    virtual ISO7816Response sendNativeCommand(unsigned char instruction,
                                             const ByteVector &data) const = 0;
    void clearAuthenticationState()
    {
        sessionKey_.clear();
        sessionIV_.clear();
    }

  private:
    ByteVector sessionKey_;
    ByteVector sessionIV_;
};
}
