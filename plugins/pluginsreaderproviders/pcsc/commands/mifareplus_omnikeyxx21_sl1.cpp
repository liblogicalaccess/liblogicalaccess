//
// Created by xaqq on 7/1/15.
//

#include "mifareplus_omnikeyxx21_sl1.hpp"
#include "logicalaccess/crypto/aes_helper.hpp"
#include "logicalaccess/crypto/lla_random.hpp"
#include <cassert>


using namespace logicalaccess;

bool MifarePlusOmnikeyXX21SL1Commands::AESAuthenticate(std::shared_ptr<AES128Key> key,
                                                       uint16_t keyslot)
{
    LLA_LOG_CTX("AES Authentication Mifare Plus SL1 and OKXX21");

    GenericSessionGuard gsg(this);
    return MifarePlusSL1Commands::AESAuthenticate(key, keyslot);
}

MifarePlusOmnikeyXX21SL1Commands::GenericSessionGuard::GenericSessionGuard(Commands *cmd)
    : cmd_(cmd)
{
    // The reader card adapter shall be a PCSCReaderCardAdapter, because this
    // generic guard is only for PCSC command (and only for XX21).
    rca_ = std::dynamic_pointer_cast<PCSCReaderCardAdapter>(cmd_->getReaderCardAdapter());
    assert(rca_);

    adapter_ = std::make_shared<Adapter>();
    adapter_->setDataTransport(rca_->getDataTransport());
    adapter_->setResultChecker(rca_->getResultChecker());
    cmd->setReaderCardAdapter(adapter_);

    rca_->sendAPDUCommand(0xFF, 0xA0, 0x00, 0x07, 0x03, {0x01, 0x00, 0x01});
}

MifarePlusOmnikeyXX21SL1Commands::GenericSessionGuard::~GenericSessionGuard()
{
    cmd_->setReaderCardAdapter(rca_);
    rca_->sendAPDUCommand(0xFF, 0xA0, 0x00, 0x07, 0x03, {0x01, 0x00, 0x02});
}

ByteVector MifarePlusOmnikeyXX21SL1Commands::GenericSessionGuard::Adapter::adaptCommand(
    const ByteVector &in)
{
    // We have to build the full PCSC Command.
    ByteVector pcsc_header = {0xFF, 0xA0, 0x00, 0x05,
                              static_cast<uint8_t>(6 + in.size())};

    ByteVector full_cmd = {0x01, 0x00, 0xF3, 0x00, 0x00, 0x64};
    full_cmd.insert(full_cmd.end(), in.begin(), in.end());
    full_cmd.push_back(0);

    full_cmd.insert(full_cmd.begin(), pcsc_header.begin(), pcsc_header.end());
    return full_cmd;
}

ByteVector MifarePlusOmnikeyXX21SL1Commands::GenericSessionGuard::Adapter::adaptAnswer(
    const ByteVector &answer)
{
    LLA_LOG_CTX("ADAPTER");
    LOG(DEBUGS) << "BEFORE ADAPTING: " << answer;
    ByteVector ret(answer.begin() + 2, answer.end());
    return ret;
}
