/**
 * \file tcpdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief TCP data transport.
 */

#include "logicalaccess/readerproviders/tcpdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{
	TcpDataTransport::TcpDataTransport()
	{
		d_ipAddress = "127.0.0.1";
		d_port = 23;
	}

	TcpDataTransport::~TcpDataTransport()
	{
	}

	std::string TcpDataTransport::getIpAddress() const
	{
		return d_ipAddress;
	}

	void TcpDataTransport::setIpAddress(std::string ipAddress)
	{
		d_ipAddress = ipAddress;
	}

	int TcpDataTransport::getPort() const
	{
		return d_port;
	}

	void TcpDataTransport::setPort(int port)
	{
		d_port = port;
	}

	boost::shared_ptr<boost::asio::ip::tcp::socket> TcpDataTransport::getSocket()
	{
		return d_socket;
	}

	bool TcpDataTransport::connect()
	{
		if (!d_socket)
		{
			boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(getIpAddress()), 7171);
			d_socket.reset(new boost::asio::ip::tcp::socket(ios));

			try
			{
				d_socket->connect(endpoint);
			}
			catch(boost::system::system_error&)
			{
				d_socket.reset();
			}
		}

		return (d_socket);
	}

	void TcpDataTransport::disconnect()
	{
		if (d_socket)
		{
			d_socket->close();
			d_socket.reset();
		}
	}

	bool TcpDataTransport::isConnected()
	{
		return (d_socket);
	}

	std::string TcpDataTransport::getName() const
	{
		return d_ipAddress;
	}

	void TcpDataTransport::send(const std::vector<unsigned char>& data)
	{
		if (data.size() > 0)
		{
			boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getSocket();
			socket->send(boost::asio::buffer(data));
		}
	}

	std::vector<unsigned char> TcpDataTransport::receive(long int timeout)
	{
		std::vector<unsigned char> res;

		boost::shared_ptr<boost::asio::ip::tcp::socket> socket = getSocket();
		std::vector<unsigned char> bufrcv(1024);

		try
		{
			// TODO: Need to set up a timeout here !
			size_t len = socket->receive(boost::asio::buffer(bufrcv));
			res = std::vector<unsigned char>(bufrcv.begin(), bufrcv.begin() + len);
		}
		catch(...)
		{
		}

		return res;
	}

	void TcpDataTransport::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getTransportType());
		node.put("IpAddress", d_ipAddress);
		node.put("Port", d_port);

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void TcpDataTransport::unSerialize(boost::property_tree::ptree& node)
	{
		d_ipAddress = node.get_child("IpAddress").get_value<std::string>();
		d_port = node.get_child("Port").get_value<int>();
	}

	std::string TcpDataTransport::getDefaultXmlNodeName() const
	{
		return "TcpDataTransport";
	}
}
