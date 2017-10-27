/**
 * \file pcscdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC transport for reader/card commands.
 */

#ifndef LOGICALACCESS_PCSCDATATRANSPORT_HPP
#define LOGICALACCESS_PCSCDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "pcscreaderunit.hpp"
#include <boost/asio.hpp>

namespace logicalaccess
{
#define TRANSPORT_PCSC "PCSC"

/**
 * \brief An PCSC data transport class.
 */
class LIBLOGICALACCESS_API PCSCDataTransport : public DataTransport
{
  public:
    /**
     * \brief Constructor.
     */
    PCSCDataTransport();

    /**
     * \brief Destructor.
     */
    virtual ~PCSCDataTransport();

    /**
     * \brief Get the transport type of this instance.
     * \return The transport type.
     */
    std::string getTransportType() const override
    {
        return TRANSPORT_PCSC;
    }

    /**
     * \param Connect to the transport layer.
     * \return True on success, false otherwise.
     */
    bool connect() override;

    /**
     * \param Disconnect from the transport layer.
     */
    void disconnect() override;

    /**
     * \briaf Get if connected to the transport layer.
     * \return True if connected, false otherwise.
     */
    bool isConnected() override;

    /**
     * \brief Get the PC/SC reader unit.
     * \return The PC/SC reader unit.
     */
    std::shared_ptr<PCSCReaderUnit> getPCSCReaderUnit() const
    {
        return std::dynamic_pointer_cast<PCSCReaderUnit>(getReaderUnit());
    }

    /**
     * \brief Get the data transport endpoint name.
     * \return The data transport endpoint name.
     */
    std::string getName() const override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Check the card error and throw exception if needed.
     * \param errorFlag The error flag.
     */
    static void CheckCardError(unsigned int errorFlag);

    void send(const ByteVector &data) override;

    ByteVector receive(long int timeout) override;

  protected:
    bool d_isConnected;

    ByteVector d_response;
};
}

#endif /* LOGICALACCESS_PCSCDATATRANSPORT_HPP */