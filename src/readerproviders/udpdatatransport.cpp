/**
 * \file udpdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief UDP data transport.
 */

#include "logicalaccess/readerproviders/udpdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{
	UdpDataTransport::UdpDataTransport() : d_ipAddress("127.0.0.1"), d_port(9559)
	{
	}

	UdpDataTransport::~UdpDataTransport()
	{
	}

	std::string UdpDataTransport::getIpAddress() const
	{
		return d_ipAddress;
	}

	void UdpDataTransport::setIpAddress(std::string ipAddress)
	{
		d_ipAddress = ipAddress;
	}

	int UdpDataTransport::getPort() const
	{
		return d_port;
	}

	void UdpDataTransport::setPort(int port)
	{
		d_port = port;
	}

	boost::shared_ptr<boost::asio::ip::udp::socket> UdpDataTransport::getSocket() const
	{
		return d_socket;
	}

	bool UdpDataTransport::connect()
	{
		if (!d_socket)
		{
			boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(getIpAddress()), getPort());
			d_socket.reset(new boost::asio::ip::udp::socket(ios));

			try
			{
				d_socket->connect(endpoint);
			}
			catch(boost::system::system_error&)
			{
				d_socket.reset();
			}
		}

		return bool(d_socket);
	}

	void UdpDataTransport::disconnect()
	{
		if (d_socket)
		{
			d_socket->close();
			d_socket.reset();
		}
	}

	bool UdpDataTransport::isConnected()
	{
		return bool(d_socket);
	}

	std::string UdpDataTransport::getName() const
	{
		return d_ipAddress;
	}

	void UdpDataTransport::send(const std::vector<unsigned char>& data)
	{
		if (data.size() > 0)
		{
			boost::shared_ptr<boost::asio::ip::udp::socket> socket = getSocket();
			socket->send(boost::asio::buffer(data));
		}
	}

	std::vector<unsigned char> UdpDataTransport::receive(long int timeout)
	{
		std::vector<unsigned char> res;
		boost::shared_ptr<boost::asio::ip::udp::socket> socket = getSocket();

		long int currentWait = 0;
		size_t lenav = socket->available();
		while (lenav == 0 && (timeout == 0 || currentWait < timeout))
		{  

			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			currentWait += 250;

			lenav = socket->available();
		}

		if (lenav > 0)
		{
			boost::array<char, 128> recv_buf;
			boost::asio::ip::udp::endpoint sender_endpoint;
			// TODO: Need to set up a timeout here !
			size_t len = socket->receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
			res = std::vector<unsigned char>(recv_buf.begin(), recv_buf.begin() + len);
		}

		return res;
	}

	void UdpDataTransport::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getTransportType());
		node.put("IpAddress", d_ipAddress);
		node.put("Port", d_port);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void UdpDataTransport::unSerialize(boost::property_tree::ptree& node)
	{
		d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
		d_port = node.get_child("Port").get_value<int>();
	}

	std::string UdpDataTransport::getDefaultXmlNodeName() const
	{
		return "UdpDataTransport";
	}
}
