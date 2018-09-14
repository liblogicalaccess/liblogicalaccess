//
// Created by xaqq on 9/11/15.
//

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/crypto/aes_helper.hpp>
#include <logicalaccess/plugins/crypto/lla_random.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/cards/mifareplus/MifarePlusAESAuth.hpp>
#include <logicalaccess/plugins/cards/mifareplus/mifarepluschip.hpp>
#include <algorithm>

logicalaccess::MifarePlusAESAuth::MifarePlusAESAuth(
    std::shared_ptr<ReaderCardAdapter> rca)
    : rca_(rca)
{
}

bool logicalaccess::MifarePlusAESAuth::AESAuthenticate(std::shared_ptr<AES128Key> key,
                                                       uint16_t keyslot) const
{
    LOG(DEBUGS) << "Attempting AES SL1 authentication";

    using ByteVector = ByteVector;
    ByteVector command;
    ByteVector ret;

    command.resize(3);
    command[0] = 0x76;
    command[1] = static_cast<unsigned char>(keyslot & 0xFF);
    command[2] = static_cast<unsigned char>(keyslot >> 8);

    ret = rca_->sendCommand(command);
    LOG(DEBUGS) << "AES Authenticate... " << ret;
    EXCEPTION_ASSERT_WITH_LOG(ret.size() > 16, LibLogicalAccessException,
                              "Not enough data in buffer.");

    ByteVector rnd_b(ret.begin() + 1, ret.begin() + 17);
    return aes_auth_step2(
        AESHelper::AESDecrypt(rnd_b, ByteVector(key->getData(), key->getData() + 16), {}),
        key);
}

bool logicalaccess::MifarePlusAESAuth::aes_auth_step2(
    ByteVector rnd_b, std::shared_ptr<AES128Key> key) const
{
    ByteVector rnd_a = RandomHelper::bytes(16);
    ByteVector data;

    rotate(rnd_b.begin(), rnd_b.begin() + 1, rnd_b.end());
    data.insert(data.end(), rnd_a.begin(), rnd_a.end());
    data.insert(data.end(), rnd_b.begin(), rnd_b.end());

    ByteVector result =
        AESHelper::AESEncrypt(data, ByteVector(key->getData(), key->getData() + 16), {});
    ByteVector command;

    command.push_back(0x72);
    command.insert(command.end(), result.begin(), result.end());

    ByteVector ret;
    ret = rca_->sendCommand(command);
    LOG(DEBUGS) << "AES Authenticate STEP 2... " << ret;
    EXCEPTION_ASSERT_WITH_LOG(ret.size() > 16, LibLogicalAccessException,
                              "Not enough data in buffer.");

    // make sure the result from the result is as expected.
    ByteVector rnd_a_reader(ret.begin() + 1, ret.begin() + 17);
    return aes_auth_final(rnd_a, rnd_a_reader, key);
}

bool logicalaccess::MifarePlusAESAuth::aes_auth_final(
    const ByteVector &rnd_a, const ByteVector &rnd_a_reader,
    std::shared_ptr<AES128Key> key) const
{
    // If we received garbage, the AES code may throw. This means auth failure.
    try
    {
        auto tmp_rnd_a_reader = AESHelper::AESDecrypt(
            rnd_a_reader, ByteVector(key->getData(), key->getData() + 16), {});
        rotate(tmp_rnd_a_reader.rbegin(), tmp_rnd_a_reader.rbegin() + 1,
               tmp_rnd_a_reader.rend());

        if (tmp_rnd_a_reader == rnd_a)
        {
            LOG(INFOS) << "AES Auth Success.";
            return true;
        }
        LOG(ERRORS) << "RNDA doesn't match. AES authentication failed.";
        return false;
    }
    catch (std::exception &e)
    {
        LOG(ERRORS) << "Error decrypting AES content. AES Auth failed: " << e.what();
    }
    return false;
}
