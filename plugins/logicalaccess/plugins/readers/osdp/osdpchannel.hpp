/**
 * \file osdpchannel.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP channel.
 */

#ifndef LOGICALACCESS_OSDPCHANNEL_HPP
#define LOGICALACCESS_OSDPCHANNEL_HPP

#include <logicalaccess/readerproviders/readerunit.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpsecurechannel.hpp>

namespace logicalaccess
{
enum OSDPCommandsType
{
    OSDP_NOCMD           = 0x00,
    OSDP_ACK             = 0x40, /* General Acknowledge, Nothing to Report */
    OSDP_NAK             = 0x41, /* Negative Acknowledge – SIO Comm Handler Error Response */
    OSDP_PDID            = 0x45, /* Device Identification Report */
    OSDP_PDCAP           = 0x46, /* Device Capabilities Report */
    OSDP_LSTATR          = 0x48, /* Local Status Report */
    OSDP_ISTATR          = 0x49, /* Input Status Report */
    OSDP_OSTATR          = 0x4a, /* Output Status Report */
    OSDP_RSTATR          = 0x4b, /* Reader Status Tamper Report */
    OSDP_RAW             = 0x50, /* Card Data Report, Raw Bit Array */
    OSDP_FMT             = 0x51, /* Card Data Report, Character Array */
    OSDP_KEYPAD          = 0x53, /* Keypad Data Report */
    OSDP_COM             = 0x54, /* Communication Configuration Report */
    OSDP_BIOREADR        = 0x57, /* Biometric Data */
    OSDP_BIOMATCHR       = 0x58, /* Biometric Match Result */
    OSDP_POLL            = 0x60, /* Poll */
    OSDP_ID              = 0x61, /* ID Report Request */
    OSDP_CAP             = 0x62, /* Peripheral Device Capabilities Request */
    OSDP_DIAG            = 0x63, /* Diagnostic Function Request */
    OSDP_LSTAT           = 0x64, /* Local Status Report Request */
    OSDP_ISTAT           = 0x65, /* Input Status Report Request */
    OSDP_OSTAT           = 0x66, /* Output Status Report Request */
    OSDP_RSTAT           = 0x67, /* Reader Status Tamper Report Request */
    OSDP_OUT             = 0x68, /* Output Control Command */
    OSDP_LED             = 0x69, /* Reader LED Control Command */
    OSDP_BUZ             = 0x6a, /* Reader Buzzer Control Command */
    OSDP_TEXT            = 0x6b, /* Reader Text Output Command */
    OSDP_COMSET          = 0x6e, /* Communication Configuration Command */
    OSDP_DATA            = 0x6f, /* Data Transfer Command */
    OSDP_PROMPT          = 0x71, /* Set Automatic Reader Prompt Strings */
    OSDP_BIOREAD         = 0x73, /* Scan and send biometric data */
    OSDP_BIOMATCH        = 0x74, /* Scan and match biometric data */
    OSDP_KEYSET          = 0x75, /* Encryption Key Set */
    OSDP_CHLNG           = 0x76, /* Challenge and Secure Session Initialization Request / Client's ID and Client's Random Number Reply */
    OSDP_SCRYPT          = 0x77, /* Server's Random Number and Server Cryptogram */
    OSDP_RMAC_I          = 0x78, /* Client Cryptogram Packet and the Initial R-MAC */
    OSDP_BUSY            = 0x79, /* PD Is Busy Reply */
    OSDP_FTSTAT          = 0x7a, /* File transfer status */
    OSDP_ACURXSIZE       = 0x7b, /* Maximum Acceptable Reply Size */
    OSDP_FILETRANSFER    = 0x7c, /* File transfer Command */
    OSDP_MFG             = 0x80, /* Manufacturer Specific Command */
    OSDP_MFGREP          = 0x90, /* Manufacturer Specific Reply */
    OSDP_XWR             = 0xa1, /* Extended R/W commands */
    OSDP_ABORT           = 0xa2, /* Stop Multi Part Message */
    OSDP_KEEPACTIVE      = 0xa7, /* Keep secure channel active */
    OSDP_XRD             = 0xb1  /* Extended R/W commands Reply */
};

/**
 * \brief OSDP Channel class.
 */
class LLA_READERS_OSDP_API OSDPChannel
{
  public:
    /**
            * \brief Constructor.
            */
    OSDPChannel();

    /**
            * \brief Destructor.
            */
    ~OSDPChannel()
    {
    }


    ByteVector createPackage();

    void unPackage(ByteVector result);


    void setAddress(unsigned char address)
    {
        m_address = address;
    }

    unsigned char getAddress() const
    {
        return m_address;
    }

    void setSequenceNumber(unsigned char sequenceNumber)
    {
        m_sequenceNumber = sequenceNumber;
    }

    unsigned char getSequenceNumber() const
    {
        return m_sequenceNumber;
    }

    void setData(ByteVector data)
    {
        m_data = data;
    }

    ByteVector &getData()
    {
        return m_data;
    }

    void setCommandsType(OSDPCommandsType replyType)
    {
        m_reply_type = replyType;
    }

    OSDPCommandsType getCommandsType() const
    {
        return m_reply_type;
    }

    bool isSCB;

    OSDPSecureChannelType getSecurityBlockType() const
    {
        return m_securityBlockType;
    }

    void setSecurityBlockType(OSDPSecureChannelType securityBlockType)
    {
        m_securityBlockType = securityBlockType;
    }

    void setSecurityBlockData(ByteVector data)
    {
        m_securityBlockData = data;
    }

    ByteVector &getSecurityBlockData()
    {
        return m_securityBlockData;
    }

    std::shared_ptr<OSDPSecureChannel> getSecureChannel() const
    {
        return m_secureChannel;
    }

    void setSecureChannel(std::shared_ptr<OSDPSecureChannel> securechannel)
    {
        m_secureChannel = securechannel;
    }

  private:
    unsigned char m_address;

    unsigned char m_sequenceNumber;

    ByteVector m_data;

    OSDPCommandsType m_reply_type;

    OSDPSecureChannelType m_securityBlockType;

    ByteVector m_securityBlockData;

    std::shared_ptr<OSDPSecureChannel> m_secureChannel;
};
}

#endif /* LOGICALACCESS_OSDPCHANNEL_HPP */
