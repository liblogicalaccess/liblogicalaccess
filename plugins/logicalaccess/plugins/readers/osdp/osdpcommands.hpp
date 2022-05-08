/**
 * \file osdpcommands.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP commands.
 */

#ifndef LOGICALACCESS_OSDPCOMMANDS_HPP
#define LOGICALACCESS_OSDPCOMMANDS_HPP

#include <logicalaccess/cards/commands.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpchannel.hpp>

#include <vector>
#include <functional>

namespace logicalaccess
{
#define CMD_ODSP "ODSP"

enum class TemporaryControleCode : uint8_t
{
    NOP                      = 0x00,
    CancelTemporaryOperation = 0x01,
    SetTemporaryState        = 0x02
};

enum class PermanentControlCode : uint8_t
{
    NotPermanentState = 0x00,
    SetPermanentState = 0x01
};

enum class OSDPColor : uint8_t
{
    Black = 0x00,
    Red   = 0x01,
    Green = 0x02,
    Amber = 0x03,
    Blue  = 0x04
};

enum class ReaderTamperStatus : uint8_t
{
    Normal = 0x00,
    NotConnected = 0x01,
    Tamper = 0x02
};

enum class DataFormatCode : uint8_t
{
    RawFormat = 0x00,
    WiegandFormat = 0x01
};

enum class ReadDirection : uint8_t
{
    ForwardRead = 0x00,
    ReverseRead = 0x01
};

enum class FileTransferStatus : int16_t
{
    ReadyToProceed = 0,
    Processed = 1,
    Rebooting = 2,
    Finishing = 3,
    Abort = -1,
    UnrecognizedContent = -2,
    InvalidData = -3
};

enum class BiometricType : uint8_t
{
    Default = 0x00,
    RightThumbPrint = 0x01,
    RightIndexFingerPrint = 0x02,
    RightMiddleFingerPrint = 0x03,
    RightRingFingerPrint = 0x04,
    RightLittleFingerPrint = 0x05,
    LeftThumbPrint = 0x06,
    LeftIndexFingerPrint = 0x07,
    LeftMiddleFingerPrint = 0x08,
    LeftRingFingerPrint = 0x09,
    LeftLittleFingerPrint = 0x0a,
    RightIrisScan = 0x0b,
    RightRetinaScan = 0x0c,
    LeftIrisScan = 0x0d,
    LeftRetinaScan = 0x0e,
    FullFaceImage = 0x0f,
    RightHandGeometry = 0x10,
    LeftHandGeometry = 0x11
};

enum class BiometricFormat : uint8_t
{
    Default = 0x00,
    RawPGM = 0x01,
    ANSI_Template71 = 0x02
};

enum class BiometricStatus : uint8_t
{
    Success = 0x00,
    Timeout = 0x01,
    UnknownError = 0xff    
};

typedef struct t_led_cmd
{
    uint8_t reader;
    uint8_t ledNumber;
    TemporaryControleCode tempControlCode;
    uint8_t tempOnTime;
    uint8_t tempOffTime;
    OSDPColor tempOnColor;
    OSDPColor tempOffColor;
    uint8_t timerLSB;
    uint8_t timerMSB;
    PermanentControlCode permControlCode;
    uint8_t permOnTime;
    uint8_t permOffTime;
    OSDPColor permOnColor;
    OSDPColor permOffColor;
} s_led_cmd;

typedef struct t_buz_cmd
{
    uint8_t reader;
    uint8_t toneCode;
    uint8_t onTime;
    uint8_t offTime;
    uint8_t count;
} s_buz_cmd;

#define OSDP_CMD_TEXT_MAX_LEN 32

typedef struct t_text_cmd
{
    uint8_t reader;
    uint8_t controlCode;
    uint8_t tempTime;
    uint8_t offsetRow;
    uint8_t offsetCol;
    uint8_t length;
    uint8_t data[OSDP_CMD_TEXT_MAX_LEN];
} s_text_cmd;

typedef struct t_pdid_report
{
    uint8_t vendorCode1;
    uint8_t vendorCode2;
    uint8_t vendorCode3;
    uint8_t modelNumber;
    uint8_t version;
    uint32_t serialNumber;
    uint8_t firmwareMajor;
    uint8_t firmwareMinor;
    uint8_t firmwareBuild;
} s_pdid_report;

typedef struct t_pdcap_report
{
    uint8_t functionCode;
    uint8_t compliance;
    uint8_t numberOf;
} s_pdcap_report;

typedef struct t_lstat_report
{
    uint8_t tamperStatus;
    uint8_t powerStatus;
} s_lstat_report;

#define OSDP_EVENT_MAX_LEN 64

typedef struct t_carddata_raw
{
    uint8_t readerNumber;
    DataFormatCode formatCode;
    uint16_t bitCount;
    uint8_t data[OSDP_EVENT_MAX_LEN];
} s_carddata_raw;

typedef struct t_carddata_fmt
{
    uint8_t readerNumber;
    ReadDirection readDirection;
    uint8_t characterCount;
    uint8_t data[OSDP_EVENT_MAX_LEN];
} s_carddata_fmt;

typedef struct t_keypad
{
    uint8_t readerNumber;
    uint8_t digitCount;
    uint8_t data[OSDP_EVENT_MAX_LEN];
} s_keypad;

typedef struct t_com
{
    uint8_t address;
    uint32_t baudrate;
} s_com;

typedef struct t_ftstat
{
    uint8_t action;
    uint16_t delay;
    FileTransferStatus statusDetail;
    uint16_t updateMsgMax;
} s_ftstat;

#define OSDP_BIOTEMPLATE_MAX_LEN 512

typedef struct t_bioreadr
{
    uint8_t readerNumber;
    BiometricStatus status;
    BiometricType bioType;
    uint8_t bioQuality;
    uint8_t bioLength;
    uint8_t bioTemplate[OSDP_BIOTEMPLATE_MAX_LEN];
} s_bioreadr;

typedef struct t_biomatchr
{
    uint8_t readerNumber;
    BiometricStatus status;
    uint8_t score;
} s_biomatchr;

typedef std::function<void(uint8_t, ByteVector)> OsdpReaderEvent;
typedef std::function<void(s_bioreadr&)> OsdpBioReadEvent;
typedef std::function<void(s_biomatchr&)> OsdpBioMatchEvent;
typedef std::function<void(bool, bool)> OsdpTamperEvent;

/**
 * \brief OSDP Commands class.
 */
class LLA_READERS_OSDP_API OSDPCommands : public Commands
{
  public:
    OSDPCommands()
        : Commands(CMD_ODSP)
    {
        initCommands();
    }

    explicit OSDPCommands(std::string ct)
        : Commands(ct)
    {
        initCommands();
    }

    ~OSDPCommands()
    {
    }

    void initCommands(unsigned char address = 0);

    std::shared_ptr<OSDPChannel> poll() const;

    std::shared_ptr<OSDPChannel> challenge() const;

    std::shared_ptr<OSDPChannel> sCrypt() const;

    std::shared_ptr<OSDPChannel> led(s_led_cmd &led) const;

    std::shared_ptr<OSDPChannel> buz(s_buz_cmd &led) const;
    
    std::shared_ptr<OSDPChannel> text(s_text_cmd &text) const;
    
    s_com setCommunicationSettings(uint8_t address, uint32_t baudrate) const;
    
    s_ftstat fileTransfer(ByteVector file, uint8_t transferType = 0x01) const;
    
    s_ftstat fileTransfer(uint32_t totalSize, uint32_t offset, ByteVector fragment, uint8_t transferType = 0x01) const;

    std::shared_ptr<OSDPChannel> setProfile(unsigned char profile) const;

    std::shared_ptr<OSDPChannel> getProfile() const;
    
    s_pdid_report pdID() const;
    
    std::vector<s_pdcap_report> pdCAP() const;
    
    s_lstat_report localStatus() const;
    
    s_lstat_report localStatus(std::shared_ptr<OSDPChannel> channel) const;
    
    ByteVector inputStatus() const;
    
    ByteVector inputStatus(std::shared_ptr<OSDPChannel> channel) const;
    
    ByteVector outputStatus() const;
    
    ByteVector outputStatus(std::shared_ptr<OSDPChannel> channel) const;
    
    ReaderTamperStatus readerTamperStatus() const;
    
    ReaderTamperStatus readerTamperStatus(std::shared_ptr<OSDPChannel> channel) const;
    
    std::shared_ptr<OSDPChannel> abort() const;
    
    std::shared_ptr<OSDPChannel> bioRead(BiometricType type, BiometricFormat format, uint8_t quality) const;
    
    std::shared_ptr<OSDPChannel> bioMatch(BiometricType type, BiometricFormat format, uint8_t quality, ByteVector& bioTemplate) const;

    std::shared_ptr<OSDPChannel> disconnectFromSmartcard() const;

    void setSecureChannel(std::shared_ptr<OSDPChannel> channel)
    {
        m_channel = channel;
    }

    std::shared_ptr<OSDPChannel> getChannel() const
    {
        return m_channel;
    }

    std::shared_ptr<OSDPChannel> stransmit() const;
    
    std::shared_ptr<OSDPChannel> transmit() const;
    
    void setCardEventHandler(OsdpReaderEvent cardHandler)
    {
        handleCardEvent = cardHandler;
    }
    
    void setKeypadEventHandler(OsdpReaderEvent keypadHandler)
    {
        handleKeypadEvent = keypadHandler;
    }
    
    void setBioReadEventHandler(OsdpBioReadEvent bioreadHandler)
    {
        handleBioReadEvent = bioreadHandler;
    }
    
    void setTamperEventHandler(OsdpTamperEvent tamperHandler)
    {
        handleTamperEvent = tamperHandler;
    }

  private:
    std::shared_ptr<OSDPChannel> m_channel;
    
    OsdpReaderEvent handleCardEvent;
    
    OsdpReaderEvent handleKeypadEvent;
    
    OsdpBioReadEvent handleBioReadEvent;
    
    OsdpBioMatchEvent handleBioMatchEvent;
    
    OsdpTamperEvent handleTamperEvent;
};
}

#endif /* LOGICALACCESS_OSDPCOMMANDS_HPP */
