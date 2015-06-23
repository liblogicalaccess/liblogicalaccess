/**
 * \file mifareplusspringcardcommandssl1.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare Plus SpringCard commands SL1.
 */

#include "mifareplusspringcardcommandssl1.hpp"
#include "../pcscreaderprovider.hpp"
#include "mifarepluschip.hpp"
#include "logicalaccess/myexception.hpp"

namespace logicalaccess
{
    MifarePlusSpringCardCommandsSL1::MifarePlusSpringCardCommandsSL1()
        : MifarePlusSpringCardCommands()
    {
    }

    MifarePlusSpringCardCommandsSL1::~MifarePlusSpringCardCommandsSL1()
    {
    }

    std::shared_ptr<MifareKey> MifarePlusSpringCardCommandsSL1::ConvertKey(std::shared_ptr<Key> key)
    {
        std::shared_ptr<MifarePlusKey> keyToConvert = std::dynamic_pointer_cast<MifarePlusKey>(key);

        EXCEPTION_ASSERT_WITH_LOG(keyToConvert, std::invalid_argument, "key must be a MifarePlusKey.");

        std::shared_ptr<MifareKey> keyConverted(new MifareKey());

        if (keyToConvert->getLength() == MIFARE_PLUS_CRYPTO1_KEY_SIZE)
            keyConverted->setData(keyToConvert->getData());

        return (keyConverted);
    }

    void MifarePlusSpringCardCommandsSL1::loadKey(std::shared_ptr<Location> location, std::shared_ptr<Key> key, MifarePlusKeyType keytype)
    {
        if (keytype == KT_KEY_CRYPTO1_A)
            MifarePCSCCommands::loadKey(location, ConvertKey(key), KT_KEY_A);
        else
            MifarePCSCCommands::loadKey(location, ConvertKey(key), KT_KEY_B);
    }

    void MifarePlusSpringCardCommandsSL1::authenticate(unsigned char blockno, std::shared_ptr<KeyStorage> key_storage, MifarePlusKeyType keytype)
    {
        if (keytype == KT_KEY_CRYPTO1_A)
            MifarePCSCCommands::authenticate(blockno, key_storage, KT_KEY_A);
        else
            MifarePCSCCommands::authenticate(blockno, key_storage, KT_KEY_B);
    }

    bool MifarePlusSpringCardCommandsSL1::SwitchLevel2(std::shared_ptr<MifarePlusKey> key)
    {
        //activate T=Cl mode
        if (!TurnOnTCL())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Error while turning on the TCL mode, or TCL already in use\n");
        }

        return (MifarePlusSpringCardCommands::GenericAESAuthentication(0x9002, key, true, false));
    }

    bool MifarePlusSpringCardCommandsSL1::SwitchLevel3(std::shared_ptr<MifarePlusKey> key)
    {
        //activate T=Cl mode
        if (!TurnOnTCL())
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Error while turning on the TCL mode, or TCL already in use\n");
        }

        return (MifarePlusSpringCardCommands::GenericAESAuthentication(0x9003, key, true, false));
    }

    bool MifarePlusSpringCardCommandsSL1::AESAuthenticate(std::shared_ptr<MifarePlusKey> key)
    {
        return (MifarePlusSpringCardCommands::GenericAESAuthentication(0x9004, key, false, false));
    }
}