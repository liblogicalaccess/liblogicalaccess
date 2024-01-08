/**
 * \file twiciso7816commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Twic ISO7816 commands.
 */

#ifndef LOGICALACCESS_TWICISO7816COMMANDS_HPP
#define LOGICALACCESS_TWICISO7816COMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/twic/twiccommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/iso7816iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/lla_readers_iso7816_api.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_TWICISO7816 "TwicISO7816"

class TwicProfile;

/**
 * \brief The Twic ISO7816 commands base class.
 */
class LLA_READERS_ISO7816_API TwicISO7816Commands : public ISO7816ISO7816Commands
#ifndef SWIG
                                                    ,
                                                    public TwicCommands
#endif
{
  public:
    /**
     * \brief Constructor.
     */
    TwicISO7816Commands();

    explicit TwicISO7816Commands(std::string ct);

    /**
     * \brief Destructor.
     */
    virtual ~TwicISO7816Commands();

    /**
     * \brief Select the TWIC application.
     */
    void selectTWICApplication() override;

    /**
     * \brief Get the Unsigned Cardholder Unique Identifier.
     * \return the data.
     */
    ByteVector getUnsignedCardholderUniqueIdentifier() override;

    /**
     * \brief Get the TWIC Privacy Key.
     * \return the data.
     * \remarks Only accessible with the contact chip.
     */
    ByteVector getTWICPrivacyKey() override;

    /**
     * \brief Get the Cardholder Unique Identifier.
     * \return the data.
     */
    ByteVector getCardholderUniqueIdentifier() override;

    /**
     * \brief Get the Cardholder Fingerprints.
     * \return the data.
     */
    ByteVector getCardHolderFingerprints() override;

    /**
     * \brief Get the Security Object.
     * \return the data.
     */
    ByteVector getSecurityObject() override;

	std::shared_ptr<ISO7816Commands> getISO7816Commands() const override
    {
        auto command = std::make_shared<ISO7816ISO7816Commands>();
        command->setChip(getChip());
        command->setReaderCardAdapter(getReaderCardAdapter());
        return command;
    }
};
}

#endif