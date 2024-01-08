/**
 * \file twiccommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Twic commands.
 */

#ifndef LOGICALACCESS_TWICCOMMANDS_HPP
#define LOGICALACCESS_TWICCOMMANDS_HPP

#include <logicalaccess/cards/accessinfo.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>
#include <logicalaccess/plugins/cards/twic/twiclocation.hpp>
#include <logicalaccess/plugins/cards/twic/lla_cards_twic_api.hpp>

namespace logicalaccess
{
/**
 * \brief The Twic commands class.
 */
class LLA_CARDS_TWIC_API TwicCommands
{
  public:
    virtual ~TwicCommands() = default;

    static size_t getMinimumBytesRepresentation(size_t value);

    static size_t getMaximumDataObjectLength(int64_t dataObject);

    static size_t getMaximumTagLength(int64_t dataObject, unsigned char tag);

    static size_t getValueFromBytes(unsigned char *data, size_t datalength);

    /**
     * \brief Get the current length for a data object.
     * \param dataObject The data object.
     * \param withObjectLength Add header that describe the object length to the total
     * number, or not.
     * \return The current data object length.
     */
    size_t getDataObjectLength(int64_t dataObject, bool withObjectLength = false);

    static bool getTagData(std::shared_ptr<TwicLocation> location, void *data,
                           size_t datalen, void *datatag, size_t &datataglen);

    /**
     * \brief Select the TWIC application.
     */
    virtual void selectTWICApplication() = 0;

    /**
     * \brief Get the Unsigned Cardholder Unique Identifier.
     * \param data The data buffer.
     * \param dataLength The buffer length.
     * \return True on success, false otherwise.
     */
    virtual ByteVector getUnsignedCardholderUniqueIdentifier() = 0;

    /**
     * \brief Get the TWIC Privacy Key.
     * \param data The data buffer.
     * \param dataLength The buffer length.
     * \return True on success, false otherwise.
     * \remarks Only accessible with the contact chip.
     */
    virtual ByteVector getTWICPrivacyKey() = 0;

    /**
     * \brief Get the Cardholder Unique Identifier.
     * \param data The data buffer.
     * \param dataLength The buffer length.
     * \return True on success, false otherwise.
     */
    virtual ByteVector getCardholderUniqueIdentifier() = 0;

    /**
     * \brief Get the Cardholder Fingerprints.
     * \param data The data buffer.
     * \param dataLength The buffer length.
     * \return True on success, false otherwise.
     */
    virtual ByteVector getCardHolderFingerprints() = 0;

    /**
     * \brief Get the Security Object.
     * \param data The data buffer.
     * \param dataLength The buffer length.
     * \return True on success, false otherwise.
     */
    virtual ByteVector getSecurityObject() = 0;

	virtual std::shared_ptr<ISO7816Commands> getISO7816Commands() const
    {
        return std::shared_ptr<ISO7816Commands>();
    }
};
}

#endif