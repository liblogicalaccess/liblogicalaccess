//
// Created by xaqq on 7/2/15.
//

#include <openssl/rand.h>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/plugins/crypto/lla_random.hpp>

std::mutex logicalaccess::RandomHelper::mutex_;

uint8_t logicalaccess::RandomHelper::byte()
{
    std::unique_lock<std::mutex> ul(mutex_);
    uint8_t random_byte;
    int rc = RAND_bytes(&random_byte, 1);

    EXCEPTION_ASSERT_WITH_LOG(rc == 1, LibLogicalAccessException,
                              "RAND_bytes failed. Cannot generate random bytes.");
    return random_byte;
}

ByteVector logicalaccess::RandomHelper::bytes(size_t size)
{
    std::unique_lock<std::mutex> ul(mutex_);
    ByteVector ret(size);

    int rc = RAND_bytes(&ret[0], (int)(size));

    EXCEPTION_ASSERT_WITH_LOG(rc == 1, LibLogicalAccessException,
                              "RAND_bytes failed. Cannot generate random bytes.");
    return ret;
}
