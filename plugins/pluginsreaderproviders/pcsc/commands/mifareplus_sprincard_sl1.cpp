//
// Created by xaqq on 7/1/15.
//

#include "mifareplus_sprincard_sl1.hpp"
#include "logicalaccess/crypto/aes_helper.hpp"
#include "logicalaccess/crypto/lla_random.hpp"
#include <cassert>

using namespace logicalaccess;

bool MifarePlusSpringcardSL1Commands::AESAuthenticate(std::shared_ptr<AES128Key> key,
                                                      uint16_t keyslot)
{
    LLA_LOG_CTX("AES Authentication Mifare Plus (keyslot: "
                << std::hex << (keyslot & 0xFF) << "-" << (keyslot >> 8)
                << ") and Springcard");
    return MifarePlusSL1Commands::AESAuthenticate(key, keyslot);
}

bool MifarePlusSpringcardSL1Commands::AESAuthenticateSL1(std::shared_ptr<AES128Key> ptr)
{
    EncapsulateGuard eg(this, true);
    return MifarePlusSL1Commands::AESAuthenticateSL1(ptr);
}

bool MifarePlusSpringcardSL1Commands::switchLevel3(std::shared_ptr<AES128Key> key)
{
    LLA_LOG_CTX("Switching MFP to Level 3 (Springcard Commands)");

    ByteVector activateT_CL;

    activateT_CL.resize(5);
    activateT_CL[0] = 0xFF;
    activateT_CL[1] = 0xFB;
    activateT_CL[2] = 0x20;
    activateT_CL[3] = 0x01;
    activateT_CL[4] = 0x00;

    ByteVector returnT_CL = getReaderCardAdapter()->sendCommand(activateT_CL, 1000);

    {
        EncapsulateGuard eg(this, false);
        return MifarePlusSL1Commands::switchLevel3(key);
    }
}

EncapsulateGuard::EncapsulateGuard(Commands *cmd, bool level_1)
    : cmd_(cmd)
{
    // The reader card adapter shall be a PCSCReaderCardAdapter, because this
    // generic guard is only for PCSC command
    rca_ = std::dynamic_pointer_cast<PCSCReaderCardAdapter>(cmd_->getReaderCardAdapter());
    assert(rca_);

    adapter_ = std::make_shared<Adapter>();
    adapter_->setDataTransport(rca_->getDataTransport());
    adapter_->setResultChecker(rca_->getResultChecker());
    adapter_->level_1_ = level_1;
    cmd->setReaderCardAdapter(adapter_);
}

EncapsulateGuard::~EncapsulateGuard()
{
    cmd_->setReaderCardAdapter(rca_);
}

ByteVector EncapsulateGuard::Adapter::adaptCommand(const ByteVector &in)
{
    ByteVector full_cmd;

    // We have to build the full PCSC Command.
    ByteVector pcsc_header;

    if (level_1_)
    {
        pcsc_header = {0xFF, 0xFE, 0x01, 0x07, static_cast<uint8_t>(in.size())};
    }
    else
    {
        pcsc_header = {0xFF, 0xFE, 0x00, 0x07, static_cast<uint8_t>(in.size())};
    }

    full_cmd.insert(full_cmd.end(), in.begin(), in.end());
    full_cmd.insert(full_cmd.begin(), pcsc_header.begin(), pcsc_header.end());
    return full_cmd;
}

ByteVector EncapsulateGuard::Adapter::adaptAnswer(const ByteVector &answer)
{
    LLA_LOG_CTX("ADAPTER");
    LOG(DEBUGS) << "BEFORE ADAPTING: " << answer;
    ByteVector ret(answer.begin(), answer.end() - 2);
    return ret;
}

void MifarePlusSpringcardSL1Commands::authenticate(unsigned char /*blockno*/,
                                                   unsigned char /*keyno*/,
                                                   MifareKeyType /*keytype*/)
{
    mscc_.setChip(getChip());
    mscc_.setReaderCardAdapter(getReaderCardAdapter());

    // mscc_.authenticate(blockno, keyno, keytype);
}
