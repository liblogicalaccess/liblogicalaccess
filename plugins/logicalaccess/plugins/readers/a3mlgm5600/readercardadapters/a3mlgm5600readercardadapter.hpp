/**
 * \file a3mlgm5600readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default A3MLGM5600 reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/readerproviders/iso14443readercommunication.hpp>
#include <logicalaccess/plugins/readers/a3mlgm5600/lla_readers_a3mlgm5600_api.hpp>
#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default A3MLGM5600 reader/card adapter class.
 */
class LLA_READERS_A3MLGM5600_API A3MLGM5600ReaderCardAdapter
    : public ReaderCardAdapter,
      public ISO14443ReaderCommunication
{
  public:
    /**
     * \brief Constructor.
     */
    A3MLGM5600ReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~A3MLGM5600ReaderCardAdapter();

    enum CmdFlag
    {
        STX = 0x02, /**< \brief The start value. */
        ETX = 0x03  /**< \brief The stop value. */
    };

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

    using ReaderCardAdapter::sendCommand;

    /**
     * \brief Send a command to the reader.
     * \param cmd The command code.
     * \param data The command data buffer.
     * \param timeout The command timeout.
     * \return The result of the command.
     */
    virtual ByteVector sendCommand(unsigned char cmd, const ByteVector &data,
                                   long int timeout = -1);

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

  protected:
    /**
     * \brief Calculate command checksum.
     * \param data The data to calculate checksum
     * \return The checksum.
     */
    static unsigned char calcBCC(const ByteVector &data);

    /**
     * \brief The current sequence number.
     */
    unsigned char d_seq;

    /**
     * \brief The last command response status.
     */
    unsigned char d_command_status;
};
}

#endif /* LOGICALACCESS_DEFAULTA3MLGM5600READERCARDADAPTER_HPP */
