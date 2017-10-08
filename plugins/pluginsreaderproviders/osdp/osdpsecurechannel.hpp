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
		OSDPSecureChannel(ByteVector uid, ByteVector PDChallenge, ByteVector PDCryptogram, ByteVector CPChallenge)
			: isSCBK_D(false), m_uid(uid), m_PDChallenge(PDChallenge), m_PDCryptogram(PDCryptogram), m_CPChallenge(CPChallenge) {}

		OSDPSecureChannel(): isSCBK_D(false)
		{
		}

		~OSDPSecureChannel() {}


		void deriveKey(std::shared_ptr<AES128Key> scbkkey, std::shared_ptr<AES128Key> scbkdkey);

		void computeAuthenticationData();

		void verifyMAC(ByteVector data);

		ByteVector computeMAC(ByteVector data, openssl::AESInitializationVector iv) const;

		ByteVector computePacketMAC(ByteVector data);

		ByteVector encryptData(ByteVector data, ByteVector iv) const;

		ByteVector decryptData(ByteVector data, ByteVector iv) const;

		bool isSCBK_D;

		ByteVector& getUID() { return m_uid; }

		ByteVector& getPDChallenge() { return m_PDChallenge; }

		ByteVector& getPDCryptogram() { return m_PDCryptogram; }

		ByteVector& getCPChallenge() { return m_CPChallenge; }

		ByteVector& getCPCryptogram() { return m_CPCryptogram; }

		ByteVector& getSMAC1() { return m_smac1; }

		void setSMAC1(ByteVector smac1) { m_smac1 = smac1; }

		ByteVector& getSMAC2() { return m_smac2; }

		void setSMAC2(ByteVector smac2) { m_smac2 = smac2; }

		ByteVector& getSENC() { return m_senc; }

		void setSENC(ByteVector senc) { m_senc = senc; }

		ByteVector& getRMAC() { return m_rmac; }

		void setRMAC(ByteVector rmac) { m_rmac = rmac; }

		ByteVector& getCMAC() { return m_cmac; }

		void setCMAC(ByteVector cmac) { m_cmac = cmac; }

	private:
		ByteVector m_uid;
		ByteVector m_PDChallenge;
		ByteVector m_PDCryptogram;
		ByteVector m_CPChallenge;
		ByteVector m_CPCryptogram;

		ByteVector m_smac1;
		ByteVector m_smac2;
		ByteVector m_senc;
		ByteVector m_rmac;
		ByteVector m_cmac;
	};
}

#endif /* LOGICALACCESS_OSDPSECURECHANNEL_HPP */

