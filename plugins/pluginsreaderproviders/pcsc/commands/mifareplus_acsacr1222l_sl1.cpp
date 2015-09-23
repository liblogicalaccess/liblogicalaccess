//
// Created by xaqq on 7/1/15.
//

#include "mifareplus_acsacr1222l_sl1.hpp"
#include "logicalaccess/crypto/aes_helper.hpp"
#include "logicalaccess/crypto/lla_random.hpp"
#include <cassert>

using namespace logicalaccess;

bool MifarePlusACSACR1222L_SL1Commands::AESAuthenticate(
    std::shared_ptr<logicalaccess::AES128Key> key, uint16_t keyslot)
{
    LLA_LOG_CTX("AES Authentication Mifare Plus SL1 and OKXX21");

    GenericSessionGuard gsg(this);
    return MifarePlusSL1Commands::AESAuthenticate(key, keyslot);
}

MifarePlusACSACR1222L_SL1Commands::GenericSessionGuard::GenericSessionGuard(
    Commands *cmd)
    : cmd_(cmd)
{
    // The reader card adapter shall be a PCSCReaderCardAdapter, because this
    // generic guard is only for PCSC command (and only for XX21).
    rca_ = std::dynamic_pointer_cast<PCSCReaderCardAdapter>(
        cmd_->getReaderCardAdapter());
    assert(rca_);

    adapter_ = std::make_shared<Adapter>();
    adapter_->setDataTransport(rca_->getDataTransport());
    adapter_->setResultChecker(rca_->getResultChecker());
    cmd->setReaderCardAdapter(adapter_);

    //rca_->sendAPDUCommand(0xFF, 0xA0, 0x00, 0x07, 0x03, {0x01, 0x00, 0x01});
}

MifarePlusACSACR1222L_SL1Commands::GenericSessionGuard::~GenericSessionGuard()
{
    cmd_->setReaderCardAdapter(rca_);
    //rca_->sendAPDUCommand(0xFF, 0xA0, 0x00, 0x07, 0x03, {0x01, 0x00, 0x02});
}

std::vector<unsigned char>
MifarePlusACSACR1222L_SL1Commands::GenericSessionGuard::Adapter::adaptCommand(
    const std::vector<unsigned char> &in)
{
    ByteVector full_cmd;

    // We have to build the full PCSC Command.
    ByteVector pcsc_header = {0xE0, 0x00, 0x00, 0x24,
                              static_cast<uint8_t>(in.size())};

    full_cmd.insert(full_cmd.begin(), pcsc_header.begin(), pcsc_header.end());
    full_cmd.insert(full_cmd.end(), in.begin(), in.end());
    //full_cmd.push_back(0);

    return full_cmd;
}

std::vector<unsigned char>
MifarePlusACSACR1222L_SL1Commands::GenericSessionGuard::Adapter::adaptAnswer(
    const std::vector<unsigned char> &answer)
{
	LLA_LOG_CTX("ADAPTER");
	LOG(DEBUGS) << "BEFORE ADAPTING: " << answer;
    ByteVector ret(answer.begin() + 2, answer.end());
    return ret;
}
