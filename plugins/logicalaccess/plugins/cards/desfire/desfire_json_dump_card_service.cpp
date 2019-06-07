//
// Created by xaqq on 4/16/19.
//

#include <logicalaccess/cards/samkeystorage.hpp>
#include <nlohmann/json.hpp>
#include <logicalaccess/myexception.hpp>
#include "desfire_json_dump_card_service.hpp"
#include "desfirekey.hpp"
#include "desfireaccessinfo.hpp"
#include "desfirelocation.hpp"

namespace logicalaccess
{

DESFireJsonDumpCardService::~DESFireJsonDumpCardService() {}

DESFireJsonDumpCardService::DESFireJsonDumpCardService(const std::shared_ptr<Chip> &chip)
    : JsonDumpCardService(chip)
{
}
std::string convert_to_spaced_key(std::string key)
{
    std::string newkeystring;
    for (size_t x = 0; x < key.length(); ++x)
    {
        newkeystring += key[x];
        if (x % 2 != 0 && x + 1 < key.length())
            newkeystring += " ";
    }
    return newkeystring;
}

std::shared_ptr<Key> DESFireJsonDumpCardService::create_key(const nlohmann::json &keyJson)
{
    std::string newkeystring = convert_to_spaced_key(keyJson.at("key_value"));

    const std::map<std::string, DESFireKeyType> keyType = {
        {"AES", DESFireKeyType::DF_KEY_AES},
        {"3K3DES", DESFireKeyType::DF_KEY_3K3DES},
        {"DES", DESFireKeyType::DF_KEY_DES}};

    auto key = std::make_shared<DESFireKey>();
    if (keyJson.at("is_sam").get<bool>())
    {
        auto sst = std::make_shared<SAMKeyStorage>();
        sst->setKeySlot(keyJson.at("sam_key_number"));
        key->setKeyStorage(sst);
    }
    else
    {
        key = std::make_shared<DESFireKey>(newkeystring);
    }
    key->setKeyVersion(keyJson.at("key_version"));
    key->setKeyType(keyType.at(keyJson.at("key_type")));

    return key;
}

void DESFireJsonDumpCardService::configure_format_infos(const nlohmann::json &json)
{

    for (const auto &app : json.at("applications"))
    {
        std::vector<std::shared_ptr<DESFireAccessInfo>> ais;

        for (const auto &file : app.at("files"))
        {
            auto fi = std::make_shared<FormatInfos>();

            auto location = std::make_shared<DESFireLocation>();
            auto ai       = std::make_shared<DESFireAccessInfo>();

            location->aid = std::stoi(app.at("appid").get<std::string>(), nullptr, 16);

            if (file.at("free_read_key").get<bool>())
            {
                location->securityLevel = EncryptionMode::CM_PLAIN;
            }
            else
            {
                location->securityLevel = EncryptionMode::CM_ENCRYPT;
                ai->readKeyno = file.at("read_key");
            }
            location->file          = file.at("fileno");
            ais.push_back(ai);

            fi->setAiToUse(ai);
            fi->setLocation(location);
            fi->setFormat(formats_.at(file.at("format")));

            // Naming convention. APPNAME.FILE_NUMBER
            std::stringstream tree;
            tree << app.at("name").get<std::string>() << "." << file.at("fileno");

            if (format_infos_.find(tree.str()) != format_infos_.end())
                throw LibLogicalAccessException(tree.str() + "Already exists.");
            format_infos_[tree.str()] = fi;
        }

        // Then assign the access info read key to a real key object.
        for (const auto &ai : ais)
        {
            for (const auto &key : app.at("keys"))
            {
                if (ai->readKeyno == key.at("key_no"))
                {
                    ai->readKey =
                        std::dynamic_pointer_cast<DESFireKey>(keys_.at(key.at("key")));
                }
            }
        }
    }
}
}