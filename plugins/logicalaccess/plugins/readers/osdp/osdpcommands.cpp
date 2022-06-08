/**
 * \file osdpreaderprovider.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief Reader Provider OSDP.
 */

#include <logicalaccess/plugins/readers/osdp/osdpcommands.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/tomcrypt.h>
#include <openssl/rand.h>
#include <thread>
#include <ctime>
#include <algorithm>

namespace logicalaccess
{
void OSDPCommands::initCommands(unsigned char address, bool installMode)
{
    m_channel.reset(new OSDPChannel());
    m_channel->setAddress(address);
    m_channel->setInstallMode(installMode);
}

std::shared_ptr<OSDPChannel> OSDPCommands::poll() const
{
    m_channel->setData(ByteVector());
    m_channel->setCommandsType(OSDP_POLL);

    auto poll = stransmit();

    LOG(LogLevel::INFOS) << "Reader poll command: " << std::hex
                         << poll->getCommandsType();

    ByteVector &data = poll->getData();
    if (poll->getCommandsType() == OSDP_XRD)
    {
        ByteVector tmp_csn;
        if (data.size() > 2 && data[1] == 0x01) // osdp_PRES
        {
            if (handleCardEvent != nullptr)
            {
                handleCardEvent(data[2], ByteVector(), 0);
            }
        }
        else if (data.size() >= 5 && data[1] == 0x02) // osdp_PR00CIRR
        {
            if (data[5] > 0 && data.size() >= 6 + data[5])
            {
                tmp_csn = ByteVector(&data[6], &data[6] + data[5]);
            }
            if (handleCardEvent != nullptr)
            {
                handleCardEvent(data[2], tmp_csn, static_cast<uint16_t>(tmp_csn.size() * 8));
            }
        }
    }
    else if (poll->getCommandsType() == OSDP_RAW)
    {
        if (data.size() >= sizeof(t_carddata_raw) - OSDP_EVENT_MAX_LEN && data.size() <= sizeof(t_carddata_raw))
        {
            s_carddata_raw carddata;
            memcpy(&carddata, &data[0], data.size());
            
            if (carddata.bitCount > 0)
            {
                uint16_t byteCount = (carddata.bitCount + 7) / 8;
                if (byteCount <= OSDP_EVENT_MAX_LEN)
                {
                    ByteVector tmp_csn(carddata.data, carddata.data + byteCount);
                    if (handleCardEvent != nullptr)
                    {
                        handleCardEvent(carddata.readerNumber, tmp_csn, carddata.bitCount);
                    }
                }
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Bad buffer size for OSDP_RAW response.";
        }
    }
    else if (poll->getCommandsType() == OSDP_FMT)
    {
        if (data.size() >= sizeof(t_carddata_fmt) - OSDP_EVENT_MAX_LEN && data.size() <= sizeof(t_carddata_fmt))
        {
            s_carddata_fmt carddata;
            memcpy(&carddata, &data[0], data.size());
            
            if (carddata.characterCount > 0 && carddata.characterCount <= OSDP_EVENT_MAX_LEN)
            {
                ByteVector tmp_csn(carddata.data, carddata.data + carddata.characterCount);
                if (carddata.readDirection == ReadDirection::ReverseRead)
                {
                    std::reverse(tmp_csn.begin(), tmp_csn.end());
                }
                if (handleCardEvent != nullptr)
                {
                    handleCardEvent(carddata.readerNumber, tmp_csn, static_cast<uint16_t>(tmp_csn.size() * 8));
                }
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Bad buffer size for OSDP_FMT response.";
        }
    }
    else if (poll->getCommandsType() == OSDP_LSTATR)
    {
        auto lstatr = localStatus(poll);
        LOG(LogLevel::INFOS) << "Tamper status changed to: "
                             << static_cast<bool>(lstatr.tamperStatus != 0);
        handleTamperEvent(static_cast<bool>(lstatr.tamperStatus != 0), static_cast<bool>(lstatr.powerStatus != 0));
    }
    else if (poll->getCommandsType() == OSDP_KEYPAD)
    {
        if (data.size() >= sizeof(t_keypad) - OSDP_EVENT_MAX_LEN && data.size() <= sizeof(t_keypad))
        {
            s_keypad keypad;
            memcpy(&keypad, &data[0], data.size());
            
            if (keypad.digitCount > 0 && keypad.digitCount <= OSDP_EVENT_MAX_LEN)
            {
                if (handleKeypadEvent != nullptr)
                {
                    handleKeypadEvent(
                        keypad.readerNumber,
                        ByteVector(keypad.data, keypad.data + keypad.digitCount),
                        static_cast<uint16_t>(keypad.digitCount * 8));
                }
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Bad buffer size for OSDP_KEYPAD response.";
        }
    }
    else if (poll->getCommandsType() == OSDP_BIOREADR)
    {
        if (data.size() >= sizeof(t_bioreadr) - OSDP_BIOTEMPLATE_MAX_LEN && data.size() <= sizeof(t_bioreadr))
        {
            s_bioreadr bioreadr;
            memcpy(&bioreadr, &data[0], data.size());
            
            if (bioreadr.bioLength > 0 && bioreadr.bioLength <= OSDP_BIOTEMPLATE_MAX_LEN)
            {
                if (handleBioReadEvent != nullptr)
                {
                    handleBioReadEvent(bioreadr);
                }
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Bad buffer size for OSDP_BIOREADR response.";
        }
    }
    else if (poll->getCommandsType() == OSDP_BIOMATCHR)
    {
        if (data.size() == sizeof(t_biomatchr))
        {
            s_biomatchr biomatchr;
            memcpy(&biomatchr, &data[0], sizeof(biomatchr));
            
            if (handleBioMatchEvent != nullptr)
            {
                handleBioMatchEvent(biomatchr);
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Bad buffer size for OSDP_BIOREADR response.";
        }
    }
    
    return poll;
}

std::shared_ptr<OSDPChannel> OSDPCommands::challenge() const
{
    m_channel->setCommandsType(OSDP_CHLNG);
    m_channel->isSCB = true;

    RAND_seed(m_channel.get(), sizeof(*(m_channel.get())));
    EXCEPTION_ASSERT_WITH_LOG(RAND_status() == 1, LibLogicalAccessException,
                              "Insufficient entropy source");
    ByteVector rnda(8);
    if (RAND_bytes(&rnda[0], static_cast<int>(rnda.size())) != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot retrieve cryptographically strong bytes");
    }

    m_channel->setSecurityBlockData(ByteVector(3));
    m_channel->setSecurityBlockType(SCS_11);

    m_channel->setData(rnda);

    transmit();

    ByteVector data = m_channel->getData();
    if (data.size() != 32)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Challenge answer is too small.");
    ByteVector uid(data.begin(), data.begin() + 8);
    ByteVector PDChallenge(data.begin() + 8, data.begin() + 16);
    ByteVector PDCryptogram(data.begin() + 16, data.begin() + 32);
    std::shared_ptr<OSDPSecureChannel> mySecureChannel(
        new OSDPSecureChannel(uid, PDChallenge, PDCryptogram, rnda));
    m_channel->setSecureChannel(mySecureChannel);

    if (m_channel->getSecurityBlockData()[0] == 0x00)
        m_channel->getSecureChannel()->isSCBK_D = true;
    else
        m_channel->getSecureChannel()->isSCBK_D = false;

    return m_channel;
}

std::shared_ptr<OSDPChannel> OSDPCommands::sCrypt() const
{
    m_channel->setCommandsType(OSDP_SCRYPT);
    m_channel->setData(m_channel->getSecureChannel()->getCPCryptogram());

    m_channel->setSecurityBlockType(SCS_13);

    ByteVector blockData(3);
    if (m_channel->getSecureChannel()->isSCBK_D)
        blockData[0] = 0x00;
    else
        blockData[0] = 0x01;
    m_channel->setSecurityBlockData(blockData);

    transmit();

    ByteVector data = m_channel->getData();
    if (data.size() != 16)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "RMAC is too small.");
    m_channel->getSecureChannel()->setRMAC(data);

    return m_channel;
}

std::shared_ptr<OSDPChannel> OSDPCommands::keySet(const ByteVector& key) const
{
    ByteVector osdpCommand;
    
    m_channel->setCommandsType(OSDP_KEYSET);
    osdpCommand.push_back(0x01); // Secure Channel Base Key
    osdpCommand.push_back(static_cast<uint8_t>(key.size()));
    osdpCommand.insert(osdpCommand.end(), key.begin(), key.end());
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    if (channel->getCommandsType() == OSDP_NAK)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "OSDP_KEYSET failed with NAK.");
    }
    else if (channel->getCommandsType() != OSDP_ACK)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Unexpected response of OSDP_KEYSET command.");
    }
    
    return channel;
}

std::shared_ptr<OSDPChannel> OSDPCommands::led(s_led_cmd &led) const
{
    ByteVector ledConfig(14);

    m_channel->setCommandsType(OSDP_LED);
    memcpy(&ledConfig[0], &led, sizeof(s_led_cmd));
    m_channel->setData(ledConfig);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::buz(s_buz_cmd &led) const
{
    ByteVector buzConfig(14);

    m_channel->setCommandsType(OSDP_BUZ);
    memcpy(&buzConfig[0], &led, sizeof(s_buz_cmd));
    m_channel->setData(buzConfig);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::text(s_text_cmd &text) const
{
    m_channel->setCommandsType(OSDP_TEXT);
    auto textptr = reinterpret_cast<unsigned char*>(&text);
    ByteVector textConfig(textptr, textptr + (sizeof(s_text_cmd) - (OSDP_CMD_TEXT_MAX_LEN - text.length)));
    m_channel->setData(textConfig);
    return stransmit();
}

s_com OSDPCommands::setCommunicationSettings(uint8_t address, uint32_t baudrate) const
{
    ByteVector osdpCommand;
    
    m_channel->setCommandsType(OSDP_COMSET);
    osdpCommand.push_back(address);
    osdpCommand.push_back(static_cast<uint8_t>(baudrate & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((baudrate >> 8) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((baudrate >> 16) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((baudrate >> 24) & 0xff));
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    if (channel->getCommandsType() != OSDP_COM)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_COM");
    }
    
    s_com com;
    ByteVector &data = channel->getData();
    if (data.size() != sizeof(com))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size do not match OSDP_COM response");
    }
    memcpy(&com, &data[0], sizeof(com));
    return com;
}

s_ftstat OSDPCommands::fileTransfer(ByteVector file, uint8_t transferType) const
{
    uint32_t offset = 0;
    s_ftstat ftstat;
    uint32_t fragmentsize = 128;
    do
    {
        uint32_t chunksize = static_cast<uint32_t>(file.size()) - offset;
        if (chunksize > fragmentsize)
        {
            chunksize = fragmentsize;
        }
        ftstat = fileTransfer(static_cast<uint32_t>(file.size()), offset, ByteVector(file.begin() + offset, file.begin() + offset + chunksize), transferType);
        if (static_cast<int16_t>(ftstat.statusDetail) >= 0)
        {
            offset += chunksize;
            if (ftstat.updateMsgMax > 0)
            {
                fragmentsize = ftstat.updateMsgMax;
            }
            if (ftstat.delay > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(ftstat.delay));
            }
        }
    } while (offset < file.size() && static_cast<int16_t>(ftstat.statusDetail) >= 0);
    return ftstat;
}

s_ftstat OSDPCommands::fileTransfer(uint32_t totalSize, uint32_t offset, ByteVector fragment, uint8_t transferType) const
{
    ByteVector osdpCommand;
    
    m_channel->setCommandsType(OSDP_FILETRANSFER);
    osdpCommand.push_back(transferType);
    osdpCommand.push_back(static_cast<uint8_t>(totalSize & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((totalSize >> 8) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((totalSize >> 16) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((totalSize >> 24) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>(offset & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((offset >> 8) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((offset >> 16) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((offset >> 24) & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>(fragment.size() & 0xff));
    osdpCommand.push_back(static_cast<uint8_t>((fragment.size() >> 8) & 0xff));
    osdpCommand.insert(osdpCommand.end(), fragment.begin(), fragment.end());
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    if (channel->getCommandsType() == OSDP_NAK)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "File Transfer ended with an OSDP_NAK response.");
    }
    else if (channel->getCommandsType() != OSDP_FTSTAT)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_FTSTAT");
    }
    
    s_ftstat ftstat;
    ByteVector &data = channel->getData();
    if (data.size() != sizeof(ftstat))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size do not match OSDP_FTSTAT response");
    }
    memcpy(&ftstat, &data[0], sizeof(ftstat));
    return ftstat;
}

std::shared_ptr<OSDPChannel> OSDPCommands::getProfile() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_XWR);
    osdpCommand.push_back(0x00); // XRW_PROFILE 0x00
    osdpCommand.push_back(0x01); // XRW_PCMND 0x01 Get Profile Setting
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::setProfile(unsigned char profile) const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_XWR);
    osdpCommand.push_back(0x00);    // XRW_PROFILE 0x00
    osdpCommand.push_back(0x02);    // XRW_PCMND 0x01 Set Profile Setting
    osdpCommand.push_back(profile); // Set profile
    //osdpCommand.push_back(0x01);    // Enable Read Card Info Report on Mode-00
    m_channel->setData(osdpCommand);
    return stransmit();
}

s_pdid_report OSDPCommands::pdID() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_ID);
    osdpCommand.push_back(0x00);
    m_channel->setData(osdpCommand);
    auto id = stransmit();
    if (id->getCommandsType() != OSDP_PDID)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_PDID");
    }
    
    s_pdid_report pdid;
    ByteVector &data = id->getData();
    if (data.size() != sizeof(pdid))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size do not match OSDP_PDID response");
    }
    
    memcpy(&pdid, &data[0], sizeof(pdid));
    return pdid;
}

std::vector<s_pdcap_report> OSDPCommands::pdCAP() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_CAP);
    osdpCommand.push_back(0x00);
    m_channel->setData(osdpCommand);
    auto cap = stransmit();
    if (cap->getCommandsType() != OSDP_PDCAP)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_PDCAP");
    }
    
    std::vector<s_pdcap_report> pdcaps;
    ByteVector &data = cap->getData();
    if ((data.size() % sizeof(t_pdcap_report)) != 0)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size is not aligned with OSDP_PDCAP response");
    }
    
    for (size_t i = 0; i < data.size(); i += sizeof(t_pdcap_report))
    {
        s_pdcap_report pdcap;
        memcpy(&pdcap, &data[0], sizeof(pdcap));
        pdcaps.push_back(pdcap);
    }
    
    return pdcaps;
}

s_lstat_report OSDPCommands::localStatus() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_LSTAT);
    osdpCommand.push_back(0x00);
    
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    return localStatus(channel);
}

s_lstat_report OSDPCommands::localStatus(std::shared_ptr<OSDPChannel> channel) const
{
    if (channel->getCommandsType() != OSDP_LSTATR)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_LSTATR");
    }
    
    s_lstat_report lstatr;
    ByteVector &data = channel->getData();
    if (data.size() != sizeof(lstatr))
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size do not match OSDP_LSTATR response");
    }
    
    memcpy(&lstatr, &data[0], sizeof(lstatr));
    return lstatr;
}

ByteVector OSDPCommands::inputStatus() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_ISTAT);
    osdpCommand.push_back(0x00);
    
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    return inputStatus(channel);
}

ByteVector OSDPCommands::inputStatus(std::shared_ptr<OSDPChannel> channel) const
{
    if (channel->getCommandsType() != OSDP_ISTATR)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_ISTATR");
    }
    
    return channel->getData();
}

ReaderTamperStatus OSDPCommands::readerTamperStatus() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_RSTAT);
    osdpCommand.push_back(0x00);
    
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    return readerTamperStatus(channel);
}

ReaderTamperStatus OSDPCommands::readerTamperStatus(std::shared_ptr<OSDPChannel> channel) const
{
    if (channel->getCommandsType() != OSDP_RSTATR)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_RSTATR");
    }
    
    if (channel->getData().size() != 1)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Data size do not match OSDP_RSTATR response");
    }
    
    return static_cast<ReaderTamperStatus>(channel->getData()[0]);
}

ByteVector OSDPCommands::outputStatus() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_OSTAT);
    osdpCommand.push_back(0x00);
    
    m_channel->setData(osdpCommand);
    auto channel = stransmit();
    return outputStatus(channel);
}

ByteVector OSDPCommands::outputStatus(std::shared_ptr<OSDPChannel> channel) const
{
    if (channel->getCommandsType() != OSDP_OSTATR)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Expected response type is OSDP_OSTATR");
    }
    
    return channel->getData();
}

std::shared_ptr<OSDPChannel> OSDPCommands::abort() const
{
    m_channel->setCommandsType(OSDP_ABORT);
    auto channel = stransmit();
    if (channel->getCommandsType() == OSDP_NAK)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "OSDP_ABORT failed with NAK.");
    }
    else if (channel->getCommandsType() != OSDP_ACK)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Unexpected response of OSDP_ABORT command.");
    }
    
    return channel;
}

std::shared_ptr<OSDPChannel> OSDPCommands::bioRead(BiometricType type, BiometricFormat format, uint8_t quality) const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_BIOREAD);
    osdpCommand.push_back(static_cast<uint8_t>(type));
    osdpCommand.push_back(static_cast<uint8_t>(format));
    osdpCommand.push_back(quality);
    
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::bioMatch(BiometricType type, BiometricFormat format, uint8_t quality, ByteVector& bioTemplate) const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_BIOMATCH);
    osdpCommand.push_back(static_cast<uint8_t>(type));
    osdpCommand.push_back(static_cast<uint8_t>(format));
    osdpCommand.push_back(quality);
    osdpCommand.push_back(static_cast<unsigned char>(bioTemplate.size() & 0xff));
    osdpCommand.push_back(static_cast<unsigned char>((bioTemplate.size() >> 8) & 0xff));
    osdpCommand.insert(osdpCommand.end(), bioTemplate.begin(), bioTemplate.end());
    
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::getPIVData(s_pivdata& data) const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_PIVDATA);
    osdpCommand.insert(osdpCommand.end(), reinterpret_cast<uint8_t*>(&data), reinterpret_cast<uint8_t*>(&data) + sizeof(data));
    
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::sendTransparentCommand(const ByteVector &command) const
{
    ByteVector osdpCommand;
    
    m_channel->setCommandsType(OSDP_XWR);
    osdpCommand.push_back(0x01); // XRW_PROFILE 0x01
    osdpCommand.push_back(0x01); // XRW_PCMND 0x01
    osdpCommand.push_back(m_channel->getAddress());
    osdpCommand.insert(osdpCommand.end(), command.begin(), command.end());
    
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::disconnectFromSmartcard() const
{
    ByteVector osdpCommand;

    m_channel->setCommandsType(OSDP_XWR);
    osdpCommand.push_back(0x01);                    // XRW_PROFILE 0x01
    osdpCommand.push_back(0x02);                    // XRW_PCMND 0x02 Connection Done
    osdpCommand.push_back(m_channel->getAddress());
    m_channel->setData(osdpCommand);
    return stransmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::stransmit() const
{
    if (m_channel->isSCB)
    {
        m_channel->setSecurityBlockData(ByteVector(2));
        m_channel->setSecurityBlockType(SCS_17); // Enable MAC and Data Security
    }
    else
    {
        EXCEPTION_ASSERT_WITH_LOG(m_channel->getInstallMode(),
                              LibLogicalAccessException,
                              "Encrypted communication is enforced. Otherwise please set to install mode.");
    }
    return transmit();
}

std::shared_ptr<OSDPChannel> OSDPCommands::transmit() const
{
    const clock_t begin_time = clock();

    do
    {
        ByteVector result =
            getReaderCardAdapter()->sendCommand(m_channel->createPackage());
        m_channel->unPackage(result);
        if (m_channel->getCommandsType() == OSDP_BUSY)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } while (m_channel->getCommandsType() == OSDP_BUSY &&
             (clock() - begin_time) / CLOCKS_PER_SEC < 2); // 3sec

    m_channel->setSequenceNumber(m_channel->getSequenceNumber() + 1);
    if (m_channel->getSequenceNumber() > 3)
        m_channel->setSequenceNumber(1);
    return m_channel;
}
}
