/**
 * \file mifareplussl1commands.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#include <cstring>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/crypto/aes_helper.hpp>
#include <logicalaccess/crypto/lla_random.hpp>
#include "mifarepluschip.hpp"
#include "logicalaccess/myexception.hpp"
#include "MifarePlusAESAuth.hpp"
#include "mifareplussl1commands.hpp"
#include <cassert>

#define PREFIX_PATTERN 0xE3
#define POLYNOM_PATTERN 0x1D

namespace logicalaccess
{
    /**
     * New implementation
     */

    bool MifarePlusSL1Commands::AESAuthenticate(std::shared_ptr<AES128Key> key, uint16_t keyslot)
    {
        LLA_LOG_CTX("MifarePlus SL1 AES Authenticate.");
        MifarePlusAESAuth auth(getReaderCardAdapter());

        return auth.AESAuthenticate(key, keyslot);
    }

    bool MifarePlusSL1Commands::switchLevel2(std::shared_ptr<AES128Key> key)
    {
        LLA_LOG_CTX("Switching card to Security Level 2");
        return AESAuthenticate(key, 0x9002);
    }

    bool MifarePlusSL1Commands::AESAuthenticateSL1(std::shared_ptr<AES128Key> key)
    {
        return AESAuthenticate(key, 0x9004);
	}

    bool MifarePlusSL1Commands::switchLevel3(std::shared_ptr<AES128Key> key)
    {
		return AESAuthenticate(key, 0x9003);
    }

    ByteVector MifarePlusSL1Commands::readBinary(unsigned char /*blockno*/,
                                                                    size_t /*len*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    void MifarePlusSL1Commands::updateBinary(unsigned char /*blockno*/,
                                                const ByteVector &/*buf*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    bool MifarePlusSL1Commands::loadKey(unsigned char /*keyno*/, MifareKeyType /*keytype*/, std::shared_ptr<MifareKey> /*key*/, bool /*vol*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
        return false;
    }

    void MifarePlusSL1Commands::loadKey(std::shared_ptr<Location> /*location*/,
		MifareKeyType /*keytype*/, std::shared_ptr<MifareKey> /*key*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    void MifarePlusSL1Commands::authenticate(unsigned char /*blockno*/, unsigned char /*keyno*/,
                                                MifareKeyType /*keytype*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    void MifarePlusSL1Commands::authenticate(unsigned char /*blockno*/,
                                                std::shared_ptr<KeyStorage> /*key_storage*/,
                                                MifareKeyType /*keytype*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    void MifarePlusSL1Commands::increment(uint8_t /*blockno*/, uint32_t /*value*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }

    void MifarePlusSL1Commands::decrement(uint8_t /*blockno*/, uint32_t /*value*/)
    {
        assert(false && "Call shall not be dispatched to here");
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Cannot call this");
    }
}
