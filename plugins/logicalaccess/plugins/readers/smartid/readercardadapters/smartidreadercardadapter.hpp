/**
 * \file smartidreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default SmartID reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/smartid/smartidreaderunit.hpp>
#include <logicalaccess/readerproviders/iso14443readercommunication.hpp>

#include <string>
#include <vector>

#include <logicalaccess/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default SmartID reader/card adapter class.
 */
class LIBLOGICALACCESS_API SmartIDReaderCardAdapter : public ISO7816ReaderCardAdapter,
                                                      public ISO14443ReaderCommunication
{
  public:
    /**
     * \brief Constructor.
     */
    SmartIDReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~SmartIDReaderCardAdapter();

    static const unsigned char DLE; /**< \brief The DLE value. */
    static const unsigned char STX; /**< \brief The STX value. */
    static const unsigned char ETX; /**< \brief The ETX value. */
    static const unsigned char NAK; /**< \brief The NAK value. */

    static const unsigned char RET_OK;          /**< \brief The RET_OK value. */
    static const unsigned char RET_OK_CHAIN;    /**< \brief The RET_OK_CHAIN value. */
    static const unsigned char RET_NOTAG;       /**< \brief The RET_NOTAG value. */
    static const unsigned char RET_FORMATERR;   /**< \brief The RET_FORMATERR value. */
    static const unsigned char RET_E2PROMERR;   /**< \brief The RET_E2PROMERR value. */
    static const unsigned char RET_ENVNOTFOUND; /**< \brief The RET_ENVNOTFOUND value. */
    static const unsigned char
        RET_NY_IMPLEMENTED; /**< \brief The RET_NY_IMPLEMENTED value. */

    using ReaderCardAdapter::sendCommand;

    /**
     * \brief Send an APDU command to the reader.
     */
    void sendAPDUCommand(const ByteVector &command, unsigned char *result,
                         size_t *resultlen) override;

    /**
     * \brief Send a command to the reader.
     * \param cmd The command code.
     * \param command The command buffer.
     * \param timeout The command timeout. Default is 2000 ms.
     * \return The result of the command.
     */
    virtual ByteVector sendCommand(unsigned char cmd, const ByteVector &command,
                                   long int timeout = 2000);

    /**
     * \brief Adapt the command to send to the reader.
     * \param command The command to send.
     * \return The adapted command to send.
     */
    ByteVector adaptCommand(const ByteVector &command) override;

    /**
     * \brief Adapt the asnwer received from the reader.
     * \param answer The answer received.
     * \return The adapted answer received.
     */
    ByteVector adaptAnswer(const ByteVector &answer) override;

    /**
     * \brief Send a halt command.
     */
    void tclHalt();

    /**
     * \brief Send a REQA command from the PCD to the PICC.
     * \return The ATQB PICC result.
     */
    ByteVector requestA() override;

    /**
     * \brief Send a RATS command from the PCD to the PICC.
     * \return The ATS PICC result.
     */
    ByteVector requestATS() override;

    /**
     * \brief Send a HLTB command from the PCD to the PICC.
     */
    void haltA() override;

    /**
     * \brief Manage collision.
     * \return The chip UID.
     */
    ByteVector anticollisionA() override;

    /**
     * \brief Send a REQB command from the PCD to the PICC.
     * \param afi The AFI value.
     * \return The ATQB PICC result.
     */
    ByteVector requestB(unsigned char afi = 0x00) override;

    /**
     * \brief Send a HLTB command from the PCD to the PICC.
     */
    void haltB() override;

    /**
     * \brief Send a attrib command from the PCD to the PICC.
     */
    void attrib() override;

    /**
     * \brief Manage collision.
     * \param afi The AFI value.
     * \return The chip UID.
     */
    ByteVector anticollisionB(unsigned char afi = 0x00) override;
};
}

#endif /* LOGICALACCESS_DEFAULTSMARTIDREADERCARDADAPTER_HPP */