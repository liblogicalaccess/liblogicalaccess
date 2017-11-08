/**
 * \file rplethdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth transport for reader/card commands.
 */

#ifndef LOGICALACCESS_RPLETHDATATRANSPORT_HPP
#define LOGICALACCESS_RPLETHDATATRANSPORT_HPP

#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <boost/asio.hpp>
#include <list>

namespace logicalaccess
{
#define TRANSPORT_RPLETH "Rpleth"

/**
 * \brief An rpleth data transport class.
 */
class LIBLOGICALACCESS_API RplethDataTransport : public TcpDataTransport
{
  public:
    /**
     * \brief Constructor.
     */
    RplethDataTransport();

    /**
     * \brief Destructor.
     */
    virtual ~RplethDataTransport();

    /**
     * \brief Get the transport type of this instance.
     * \return The transport type.
     */
    std::string getTransportType() const override
    {
        return TRANSPORT_RPLETH;
    }

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Send the data without computation.
     * \param data The data to send.
     */
    void sendll(const ByteVector &data);

    /**
     * \brief Send the data using rpleth protocol computation.
     * \param data The data to send.
     */
    void send(const ByteVector &data) override;

    /**
     * \brief Receive data from reader.
     * \param timeout The time to wait data.
     *        If timeout is -1, use the timeout from the Settings.
     * \return The data from reader.
     */
    ByteVector receive(long int timeout = -1) override;

    /**
     * \brief Send the Ping packet.
     */
    void sendPing();

    /**
     * \brief Get the buffer.
     * \return The buffer.
     */
    const ByteVector &getBuffer() const
    {
        return d_buffer;
    }

    /**
     * \brief Get the badges list.
     * \return The list of badge.
     */
    std::list<ByteVector> getBadges() const
    {
        return d_badges;
    }

    /**
     * \brief Send a command to the reader.
     * \param command The command buffer.
     * \param timeout The command timeout. If timeout is -1, use timeout from settings.
     * \return the result of the command.
     */
    ByteVector sendCommand(const ByteVector &command, long int timeout = -1) override;

  protected:
    /**
     * \brief Calculate command checksum.
     * \param data The data to calculate checksum
     * \return The checksum.
     */
    static unsigned char calcChecksum(const ByteVector &data);

    /**
     * \brief d_buffer from last commands response.
     */
    ByteVector d_buffer;

    /**
     * \brief Badges received from the latest commands response.
     */
    std::list<ByteVector> d_badges;
};
}

#endif /* LOGICALACCESS_RPLETHDATATRANSPORT_HPP */