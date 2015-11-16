//
// Created by xaqq on 9/11/15.
//

#include <logicalaccess/crypto/aes_helper.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/crypto/lla_random.hpp>
#include "MifarePlusSL3Auth.hpp"
#include "mifarepluschip.hpp"
#include <cassert>
#include <algorithm>
#include <logicalaccess/crypto/cmac.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>

using namespace logicalaccess;

MifarePlusSL3Auth::MifarePlusSL3Auth(std::shared_ptr<ReaderCardAdapter> rca) :
        rca_(rca),
        write_counter_(0),
        read_counter_(0)
{

}

bool MifarePlusSL3Auth::firstAuthenticate(int sector,
                                          std::shared_ptr<AES128Key> key,
                                          logicalaccess::MifareKeyType type)
{
    using ByteVector = std::vector<unsigned char>;
    ByteVector command;
    ByteVector ret;

    command.push_back(0x70);
    uint16_t keynbr = MifarePlusChip::key_number_from_sector(sector, type);
    command.push_back(keynbr & 0xFF);
    command.push_back(keynbr >> 8);
    command.push_back(0x01);
    command.push_back(0x00);

    ret = rca_->sendCommand(command);
    LOG(DEBUGS) << "Ret from SL3 auth attempt: " << ret;
    EXCEPTION_ASSERT_WITH_LOG(ret.size() > 16, LibLogicalAccessException, "Not enough data in buffer.");

    aes_key_ = ByteVector(key->getData(), key->getData() + 16);
    rnd_b_ = {ret.begin() + 1, ret.begin() + 17};
    rnd_b_ = AESHelper::AESDecrypt(rnd_b_, aes_key_, {});

    return aes_first_auth_step2();
}

bool MifarePlusSL3Auth::aes_first_auth_step2()
{
    assert(rnd_b_.size() == 16);
    assert(aes_key_.size() == 16);

    rnd_a_ = RandomHelper::bytes(16);
    ByteVector data;

    std::rotate(rnd_b_.begin(), rnd_b_.begin() + 1, rnd_b_.end());
    data.insert(data.end(), rnd_a_.begin(), rnd_a_.end());
    data.insert(data.end(), rnd_b_.begin(), rnd_b_.end());

    ByteVector result = AESHelper::AESEncrypt(data, aes_key_, {});
    ByteVector command;

    command.push_back(0x72);
    command.insert(command.end(), result.begin(), result.end());

    ByteVector ret;
    ret = rca_->sendCommand(command);
    LOG(DEBUGS) << "AES First Authenticate STEP 2... " << ret;
    EXCEPTION_ASSERT_WITH_LOG(ret.size() > 16, LibLogicalAccessException, "Not enough data in buffer.");

    // make sure the result from the result is as expected.
    ByteVector encrypted_data(ret.begin() + 1, ret.begin() + 33);
    return aes_first_auth_final(encrypted_data);
}

bool MifarePlusSL3Auth::aes_first_auth_final(const ByteVector &encrypted_data)
{
    assert(rnd_b_.size() == 16);
    assert(rnd_a_.size() == 16);
    assert(aes_key_.size() == 16);

    // If we received garbage, the AES code may throw. This means auth failure.
    try
    {
        auto data = AESHelper::AESDecrypt(encrypted_data,
                                          aes_key_,
                                          {});

        LOG(DEBUGS) << "FROM AUTH: DATA = " << data;
        //4first byte is Transaction Identifier
        trans_id_ = ByteVector(data.begin(), data.begin() + 4);

        ByteVector rnd_a_reader(data.begin() + 4, data.begin() + 20);
        std::rotate(rnd_a_reader.rbegin(), rnd_a_reader.rbegin() + 1,
                    rnd_a_reader.rend());

        if (rnd_a_reader == rnd_a_)
        {
            LOG(INFOS) << "AES Auth Success.";
            return true;
        }
        else
        {
            LOG(ERRORS) << "RNDA doesn't match. AES authentication failed.";
            return false;
        }
    }
    catch (std::exception &e)
    {
        LOG(ERRORS) << "Error decrypting AES content. AES Auth failed: " << e.what();
    }
    return false;
}

ByteVector MifarePlusSL3Auth::deriveKEnc()
{
    assert(rnd_b_.size() == 16);
    assert(rnd_a_.size() == 16);
    assert(aes_key_.size() == 16);
    assert(trans_id_.size() == 4);

    ByteVector tmp;
    unsigned char f[5];
    unsigned char g[5];
    unsigned char h[5];
    unsigned char i[5];
    int c = 0;
    //init data buffers
    while (c < 5)
    {
        h[c] = rnd_a_[c + 4];
        i[c] = rnd_b_[c + 4];
        f[c] = rnd_a_[c + 11];
        g[c] = rnd_b_[c + 11];
        c++;
    }
    //set the session key base for Kenc
    tmp.insert(tmp.end(), f, f + 5);
    tmp.insert(tmp.end(), g, g + 5);
    tmp.resize(16);
    c = 0;
    while (c < 5)
    {
        tmp[c + 10] = (h[c] ^ i[c]);
        c++;
    }
    tmp[15] = 0x11;
    return AESHelper::AESEncrypt(tmp, aes_key_, {});
}

ByteVector MifarePlusSL3Auth::deriveKMac()
{
    assert(rnd_b_.size() == 16);
    assert(rnd_a_.size() == 16);
    assert(aes_key_.size() == 16);
    assert(trans_id_.size() == 4);

    ByteVector tmp;
    unsigned char f[5];
    unsigned char g[5];
    unsigned char h[5];
    unsigned char i[5];
    int c = 0;
    //init data buffers
    while (c < 5)
    {
        h[c] = rnd_a_[c];
        i[c] = rnd_b_[c];
        f[c] = rnd_a_[c + 7];
        g[c] = rnd_b_[c + 7];
        c++;
    }
    //set the session key base for Kmac
    tmp.insert(tmp.end(), f, f + 5);
    tmp.insert(tmp.end(), g, g + 5);
    tmp.resize(16);
    c = 0;
    while (c < 5)
    {
        tmp[c + 10] = (h[c] ^ i[c]);
        c++;
    }
    tmp[15] = 0x22;
    return AESHelper::AESEncrypt(tmp, aes_key_, {});
}

ByteVector MifarePlusSL3Auth::computeWriteMac(uint8_t command_code, uint16_t block_number, const
ByteVector &data)
{
    ByteVector to_hash;
    to_hash.push_back(command_code);
    to_hash.push_back(write_counter_ & 0xFF);
    to_hash.push_back(write_counter_ >> 8 & 0xFF);
    to_hash.insert(to_hash.end(), trans_id_.begin(), trans_id_.end());
    to_hash.push_back(block_number & 0xFF);
    to_hash.push_back(block_number >> 8 & 0xFF);
    to_hash.insert(to_hash.end(), data.begin(), data.end());

    to_hash = openssl::CMACCrypto::cmac(deriveKMac(), "aes", to_hash);

    assert(to_hash.size() == 16);
    ByteVector ret(8);
    for (int i = 0; i < 8; i++)
    {
        assert(to_hash.size() > (size_t)(i * 2 + 1));
        ret[i] = to_hash[i * 2 + 1];
    }
    return ret;
}

ByteVector MifarePlusSL3Auth::cipherWriteData(const ByteVector &in)
{
    ByteVector iv = trans_id_;

    for (int i = 0; i < 3; ++i)
    {
        iv.push_back(read_counter_ & 0xFF);
        iv.push_back(read_counter_ >> 8 & 0xFF);
        iv.push_back(write_counter_ & 0xFF);
        iv.push_back(write_counter_ >> 8 & 0xFF);
    }

    LOG(DEBUGS) << "Ciphering data: iv: " << iv;
    return AESHelper::AESEncrypt(in, deriveKEnc(), iv);
}
