/**
 * \file pcscdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC data transport.
 */

#include <logicalaccess/plugins/readers/pcsc/pcscdatatransport.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <boost/property_tree/ptree.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
PCSCDataTransport::PCSCDataTransport()
{
    d_isConnected = false;
}

PCSCDataTransport::~PCSCDataTransport()
{
}

bool PCSCDataTransport::connect()
{
    d_isConnected = true;
    return true;
}

void PCSCDataTransport::disconnect()
{
    d_isConnected = false;
}

bool PCSCDataTransport::isConnected()
{
    return d_isConnected;
}

void PCSCDataTransport::send(const ByteVector &data)
{
    LLA_LOG_CTX("PCSCDataTransport");
    d_response.clear();

    EXCEPTION_ASSERT_WITH_LOG(getPCSCReaderUnit(), LibLogicalAccessException,
                              "The PCSC reader unit object"
                              "is null. We cannot send.");
    if (!data.empty())
    {
        std::array<uint8_t, 4096> responseBuffer{};
        ULONG ulNoOfDataReceived = responseBuffer.size();
        LPCSCARD_IO_REQUEST ior  = nullptr;
        switch (getPCSCReaderUnit()->getActiveProtocol())
        {
        case SCARD_PROTOCOL_T0: ior = SCARD_PCI_T0; break;

        case SCARD_PROTOCOL_T1: ior = SCARD_PCI_T1; break;

        case SCARD_PROTOCOL_RAW: ior = SCARD_PCI_RAW; break;
        default:;
        }

        LOG(LogLevel::COMS) << "APDU command: " << BufferHelper::getHex(data);

        unsigned int errorFlag = SCardTransmit(
            getPCSCReaderUnit()->getHandle(), ior, &data[0],
            static_cast<DWORD>(data.size()), nullptr, responseBuffer.data(), &ulNoOfDataReceived);

        CheckCardError(errorFlag);
        d_response = ByteVector(responseBuffer.begin(), responseBuffer.begin() + ulNoOfDataReceived);
    }
}

ByteVector PCSCDataTransport::receive(long int /*timeout*/)
{
    ByteVector r = d_response;
    LOG(LogLevel::COMS) << "APDU response: " << BufferHelper::getHex(r);

    d_response.clear();
    return r;
}

std::string PCSCDataTransport::getName() const
{
    return getPCSCReaderUnit()->getName();
}

void PCSCDataTransport::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getTransportType());
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void PCSCDataTransport::unSerialize(boost::property_tree::ptree & /*node*/)
{
}

std::string PCSCDataTransport::getDefaultXmlNodeName() const
{
    return "PCSCDataTransport";
}

void PCSCDataTransport::CheckCardError(unsigned int errorFlag)
{
    if (errorFlag != SCARD_S_SUCCESS)
    {
        std::string error_msg = PCSCConnection::strerror(errorFlag);
        THROW_EXCEPTION_WITH_LOG(CardException, error_msg);
    }
}
}