/**
 * \file osdpchannel.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP channel. 
 */

#ifndef LOGICALACCESS_OSDPCHANNEL_HPP
#define LOGICALACCESS_OSDPCHANNEL_HPP

#include "logicalaccess/readerproviders/readerunit.hpp"
#include "osdpsecurechannel.hpp"

namespace logicalaccess
{	
	enum OSDPCommandsType
	{
		NOCMD = 0x00,
		ACK = 0x40,
		NAK = 0x41,
		RAW	= 0x50,
		POLL = 0x60,
		LED = 0x69,
		CHLNG = 0x76,
		OSCRYPT = 0x77,
		BUZ = 0x6A,
		BUSY = 0x79,
		XWR = 0xa1,
		XRD = 0xb1,
        LSTATR = 0x48,
		LSTAT = 0x64
	};

	/**
	 * \brief OSDP Channel class.
	 */
	class LIBLOGICALACCESS_API OSDPChannel
	{
	public:

		/**
			* \brief Constructor.
			*/
		OSDPChannel();

		/**
			* \brief Destructor.
			*/
		~OSDPChannel() {};


		std::vector<unsigned char> createPackage();

		void unPackage(std::vector<unsigned char> result);



		void setAddress(unsigned char address) { m_address = address; };

		unsigned char getAddress() const { return m_address; };

		void setSequenceNumber(unsigned char sequenceNumber) { m_sequenceNumber = sequenceNumber; };

		unsigned char getSequenceNumber() const { return m_sequenceNumber; };

		void setData(std::vector<unsigned char> data) { m_data = data; };

		std::vector<unsigned char>& getData() { return m_data; };

		void setCommandsType(OSDPCommandsType replyType) { m_reply_type = replyType; };

		OSDPCommandsType getCommandsType() const { return m_reply_type; };

		bool isSCB;

		OSDPSecureChannelType getSecurityBlockType() const { return m_securityBlockType; };

		void setSecurityBlockType(OSDPSecureChannelType securityBlockType) { m_securityBlockType = securityBlockType; };

		void setSecurityBlockData(std::vector<unsigned char> data) { m_securityBlockData = data; };

		std::vector<unsigned char>& getSecurityBlockData() { return m_securityBlockData; };

		std::shared_ptr<OSDPSecureChannel> getSecureChannel() const { return m_secureChannel; };

		void setSecureChannel(std::shared_ptr<OSDPSecureChannel> securechannel) { m_secureChannel = securechannel; };

	private:

		unsigned char m_address;

		unsigned char m_sequenceNumber;

		std::vector<unsigned char> m_data;

		OSDPCommandsType m_reply_type;

		OSDPSecureChannelType m_securityBlockType;

		std::vector<unsigned char> m_securityBlockData;

		std::shared_ptr<OSDPSecureChannel> m_secureChannel;
	};
}

#endif /* LOGICALACCESS_OSDPCHANNEL_HPP */

