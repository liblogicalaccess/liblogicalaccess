/**
 * \file osdpsecurechannel.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP Secure Channel. 
 */

#ifndef LOGICALACCESS_OSDPSECURECHANNEL_HPP
#define LOGICALACCESS_OSDPSECURECHANNEL_HPP

#include "logicalaccess/readerproviders/readerunit.hpp"
#include "logicalaccess/cards/aes128key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"

namespace logicalaccess
{
	enum OSDPSecureChannelType
	{
		NOSCS = 0x00,
		SCS_11 = 0x11,
		SCS_12 = 0x12,
		SCS_13 = 0x13,
		SCS_14 = 0x14,
		SCS_15 = 0x15,
		SCS_16 = 0x16,
		SCS_17 = 0x17,
		SCS_18 = 0x18
	};

	/**
	 * \brief OSDP Secure Channel class.
	 */
	class LIBLOGICALACCESS_API OSDPSecureChannel
	{
	public:
		OSDPSecureChannel(std::vector<unsigned char> uid, std::vector<unsigned char> PDChallenge, std::vector<unsigned char> PDCryptogram, std::vector<unsigned char> CPChallenge)
			: isSCBK_D(false), m_uid(uid), m_PDChallenge(PDChallenge), m_PDCryptogram(PDCryptogram), m_CPChallenge(CPChallenge) {}

		OSDPSecureChannel() {}

		~OSDPSecureChannel() {}


		void deriveKey(std::shared_ptr<AES128Key> scbkkey, std::shared_ptr<AES128Key> scbkdkey);

		void computeAuthenticationData();

		void verifyMAC(std::vector<unsigned char> data);

		std::vector<unsigned char> computeMAC(std::vector<unsigned char> data, openssl::AESInitializationVector iv);

		std::vector<unsigned char> computePacketMAC(std::vector<unsigned char> data);

		std::vector<unsigned char> encryptData(std::vector<unsigned char> data, std::vector<unsigned char> iv);

		std::vector<unsigned char> decryptData(std::vector<unsigned char> data, std::vector<unsigned char> iv);

		bool isSCBK_D;

		std::vector<unsigned char>& getUID() { return m_uid; }

		std::vector<unsigned char>& getPDChallenge() { return m_PDChallenge; }

		std::vector<unsigned char>& getPDCryptogram() { return m_PDCryptogram; }

		std::vector<unsigned char>& getCPChallenge() { return m_CPChallenge; }

		std::vector<unsigned char>& getCPCryptogram() { return m_CPCryptogram; }

		std::vector<unsigned char>& getSMAC1() { return m_smac1; }

		void setSMAC1(std::vector<unsigned char> smac1) { m_smac1 = smac1; }

		std::vector<unsigned char>& getSMAC2() { return m_smac2; }

		void setSMAC2(std::vector<unsigned char> smac2) { m_smac2 = smac2; }

		std::vector<unsigned char>& getSENC() { return m_senc; }

		void setSENC(std::vector<unsigned char> senc) { m_senc = senc; }

		std::vector<unsigned char>& getRMAC() { return m_rmac; }

		void setRMAC(std::vector<unsigned char> rmac) { m_rmac = rmac; }

		std::vector<unsigned char>& getCMAC() { return m_cmac; }

		void setCMAC(std::vector<unsigned char> cmac) { m_cmac = cmac; }

	private:
		std::vector<unsigned char> m_uid;
		std::vector<unsigned char> m_PDChallenge;
		std::vector<unsigned char> m_PDCryptogram;
		std::vector<unsigned char> m_CPChallenge;
		std::vector<unsigned char> m_CPCryptogram;

		std::vector<unsigned char> m_smac1;
		std::vector<unsigned char> m_smac2;
		std::vector<unsigned char> m_senc;
		std::vector<unsigned char> m_rmac;
		std::vector<unsigned char> m_cmac;
	};
}

#endif /* LOGICALACCESS_OSDPSECURECHANNEL_HPP */

