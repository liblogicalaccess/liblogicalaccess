/**
 * \file iso7816readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default ISO7816 reader/card adapter.
 */

#ifndef LOGICALACCESS_ISO7816READERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816READERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816response.hpp>
#include <logicalaccess/plugins/crypto/iso24727crypto.hpp>

namespace logicalaccess
{
/**
 * \brief A default ISO7816 reader/card adapter class.
 */
class LLA_CARDS_ISO7816_API ISO7816ReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(const ByteVector &data);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const ByteVector &data,
                                       unsigned char le);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const ByteVector &data);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       const ByteVector &data);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, unsigned char le);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char le);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ISO7816Response sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2);

	/**
     * \brief Send an extended APDU command to the reader.
     */
    virtual ISO7816Response sendExtendedAPDUCommand(unsigned char cla, unsigned char ins,
                                                    unsigned char p1, unsigned char p2,
                                                    unsigned short lc,
                                                    const ByteVector &data);

    /**
     * \brief Send an extended APDU command to the reader.
     */
    virtual ISO7816Response sendExtendedAPDUCommand(unsigned char cla, unsigned char ins,
                                                    unsigned char p1, unsigned char p2,
                                                    unsigned short lc,
                                                    const ByteVector &data,
                                                    unsigned short le);

    ByteVector adaptCommand(const ByteVector &command) override;

    ByteVector adaptAnswer(const ByteVector &answer) override;

    void setCrypto(std::shared_ptr<ISO24727Crypto> crypto);

  private:
    /**
     * The cryptographic object that maintain the state.
     * It is used to encrypt/decrypt APDUs.
     */
    std::shared_ptr<ISO24727Crypto> crypto_;
};
}

#endif /* LOGICALACCESS_ISO7816READERCARDADAPTER_HPP */