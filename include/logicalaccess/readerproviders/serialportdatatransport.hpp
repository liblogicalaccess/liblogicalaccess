/**
 * \file serialportdatatransport.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Serial port data transport for reader/card commands. 
 */

#ifndef LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP
#define LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP

#include "logicalaccess/readerproviders/datatransport.hpp"
#include "logicalaccess/readerproviders/serialportxml.hpp"

namespace logicalaccess
{
	#define	TRANSPORT_SERIALPORT		"SerialPort"

	/**
	 * \brief A serial port data transport class.
	 */
	class LIBLOGICALACCESS_API SerialPortDataTransport : public DataTransport
	{
		public:

			/**
			 * \brief Constructor.
			 * \param portname The serial port name.
			 */
			SerialPortDataTransport(const std::string& portname = "");

			/**
			 * \brief Destructor.
			 */
			virtual ~SerialPortDataTransport();

			/**
			 * \brief Get the transport type of this instance.
			 * \return The transport type.
			 */
			virtual std::string getTransportType() const { return TRANSPORT_SERIALPORT; };

			/**
			 * \brief Configure the current serial port.
			 */
			void configure();

			/**
			 * \brief Configure the given serial port.
			 * \param port The COM port to configure.
			 * \param retryConfiguring On error, the function wait some milliseconds and retry another time to configure the serial port.
			 */
			void configure(boost::shared_ptr<SerialPortXml> port, bool retryConfiguring);

			/**
			 * \param Connect to the transport layer.
			 * \return True on success, false otherwise.
			 */
			virtual bool connect();

			/**
			 * \param Disconnect from the transport layer.
			 */
			virtual void disconnect();

			/**
			 * \briaf Get if connected to the transport layer.
			 * \return True if connected, false otherwise.
			 */
			virtual bool isConnected();

			/**
			 * \brief Get the data transport endpoint name.
			 * \return The data transport endpoint name.
			 */
			virtual std::string getName() const;

			/**
			 * \brief Start to auto-detect the first serial port with a reader. Update serial port when found.
			 */
			virtual void startAutoDetect();

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

			/**
			 * \brief Get the serial port baudrate.
			 * \return The Baudrate.
			 */
			unsigned long getPortBaudRate() const { return d_portBaudRate; };

			/**
			 * \brief Set the serial port baudrate.
			 * \param baudRate The Baudrate (9600, 19200, 38400, 57600, 115200).
			 */
			void setPortBaudRate(unsigned long baudRate) { d_portBaudRate = baudRate; };

			boost::shared_ptr<SerialPortXml> getSerialPort() const { return d_port; };

			virtual void setSerialPort(boost::shared_ptr<SerialPortXml> port) { d_port = port; };

			virtual void send(const std::vector<unsigned char>& data);

			virtual std::vector<unsigned char> receive(long int timeout);

		protected:

			/**
			 * \brief The auto-detected status
			 */
			bool d_isAutoDetected;

			/**
			 * \brief The serial port.
			 */
			boost::shared_ptr<SerialPortXml> d_port;

			/**
			 * \brief The baudrate to use when configuring the serial port.
			 */
			unsigned long d_portBaudRate;
	};

}

#endif /* LOGICALACCESS_SERIALPORTDATATRANSPORT_HPP */

