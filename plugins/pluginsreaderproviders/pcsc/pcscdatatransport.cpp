/**
 * \file pcscdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC data transport.
 */

#include "pcscdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>
#include <logicalaccess/logs.hpp>
#include <boost/property_tree/ptree.hpp>
#include "logicalaccess/myexception.hpp"

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

    void PCSCDataTransport::send(const std::vector<unsigned char>& data)
    {
        d_response.clear();

        EXCEPTION_ASSERT_WITH_LOG(getPCSCReaderUnit(), LibLogicalAccessException, "The PCSC reader unit object"
                "is null. We cannot send.");
        if (data.size() > 0)
        {
            unsigned char returnedData[255];
            ULONG ulNoOfDataReceived = sizeof(returnedData);
            LPCSCARD_IO_REQUEST ior = NULL;
            switch (getPCSCReaderUnit()->getActiveProtocol())
            {
            case SCARD_PROTOCOL_T0:
                ior = SCARD_PCI_T0;
                break;

            case SCARD_PROTOCOL_T1:
                ior = SCARD_PCI_T1;
                break;

            case SCARD_PROTOCOL_RAW:
                ior = SCARD_PCI_RAW;
                break;
            }

            LOG(LogLevel::COMS) << "APDU command: " << BufferHelper::getHex(data);

            unsigned int errorFlag = SCardTransmit(getPCSCReaderUnit()->getHandle(), ior, &data[0], static_cast<DWORD>(data.size()), NULL, returnedData, &ulNoOfDataReceived);

            CheckCardError(errorFlag);
            d_response = std::vector<unsigned char>(returnedData, returnedData + ulNoOfDataReceived);
        }
    }

    std::vector<unsigned char> PCSCDataTransport::receive(long int /*timeout*/)
    {
        std::vector<unsigned char> r = d_response;
        LOG(LogLevel::COMS) << "APDU response: " << BufferHelper::getHex(r);

        d_response.clear();
        return r;
    }

    std::string PCSCDataTransport::getName() const
    {
        return getPCSCReaderUnit()->getName();
    }

    void PCSCDataTransport::serialize(boost::property_tree::ptree& parentNode)
    {
        boost::property_tree::ptree node;

        node.put("<xmlattr>.type", getTransportType());
        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void PCSCDataTransport::unSerialize(boost::property_tree::ptree& /*node*/)
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