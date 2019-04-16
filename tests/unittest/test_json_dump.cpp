#include "logicalaccess/services/json/json_dump_card_service.hpp"
#include "nlohmann/json.hpp"
#include <gtest/gtest.h>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/plugins/cards/desfire/desfire_json_dump_card_service.hpp>

using namespace logicalaccess;

TEST(json_dump, invalid_input)
{
    DESFireJsonDumpCardService jdcs(nullptr);

    ASSERT_THROW(jdcs.dump("NOT JSON"), nlohmann::json::exception);
}

TEST(json_dump, invalid_input_wrong_format)
{
    DESFireJsonDumpCardService jdcs(nullptr);

    ASSERT_THROW(jdcs.dump("{}"), LibLogicalAccessException);
}

TEST(test_json_dump, hello)
{
    std::string card_template = R"({
            "master_card_key": "",
            "transport_key": "",
            "sam_unlock_key": "",
            "sam_unlock_key_version": null,
            "sam_unlock_key_slot": null,
            "applications": [
                {
                    "internal_id": "FE96EC9C0A17B3DC770DB44A7C35CD9F35E85872672F6BF530",
                    "name": "MY_APP",
                    "appid": "000521",
                    "crypto_type": "AES",
                    "files": [
                        {
                            "internal_id": "F1E26F97D003DC7616E8572A07C98D77F9CAE3911209C82841",
                            "fileno": 0,
                            "format": "r7Xhf9pByzRFwQk3X",
                            "read_key": 0,
                            "write_key": 0,
                            "free_read_key": false,
                            "free_write_key": false
                        }
                    ],
                    "keys": [
                        {
                            "internal_id": "BE2D400A08358DE2490A2B9B15F93BA99CA5BBC5131946DFB2",
                            "key_no": 0,
                            "key": "uuuGAg2RGniajkiEP"
                        }
                    ],
                    "optional_creation": false,
                    "auth_with_target_key": false
                }
            ],
            "data_source": {
                "type": "none",
                "manuallycreate": false,
                "manualinput": true
            },
            "encoding_cost": 1,
            "format_first": true,
            "donotsync": false,
            "formats": {
                "r7Xhf9pByzRFwQk3X": {
                    "name": "islog-presentation",
                    "fields": [
                        {
                            "internal_id": "EA8B056137B6E0F",
                            "name": "uid",
                            "len": 32,
                            "type": "FIELD_TYPE_NUMBER",
                            "default_value": "",
                            "is_identifier": true
                        },
                        {
                            "internal_id": "822454A5681AD15",
                            "name": "firstname",
                            "len": 256,
                            "type": "FIELD_TYPE_STRING",
                            "default_value": "",
                            "is_identifier": false
                        },
                        {
                            "internal_id": "C338F38466063A5",
                            "name": "lastname",
                            "len": 256,
                            "type": "FIELD_TYPE_STRING",
                            "default_value": "",
                            "is_identifier": false
                        },
                        {
                            "internal_id": "03C3C6FCB851AC4",
                            "name": "birth",
                            "len": 256,
                            "type": "FIELD_TYPE_STRING",
                            "default_value": "",
                            "is_identifier": false
                        },
                        {
                            "internal_id": "6ABD79BFDCAFA4F",
                            "name": "access",
                            "len": 256,
                            "type": "FIELD_TYPE_STRING",
                            "default_value": "",
                            "is_identifier": false
                        },
                        {
                            "internal_id": "48D97387E1A51FD",
                            "name": "photo",
                            "len": 23000,
                            "type": "FIELD_TYPE_BINARY",
                            "default_value": "",
                            "is_identifier": false
                        }
                    ]
                }
            },
            "keyValues": {
                "uuuGAg2RGniajkiEP": {
                    "key_diversification": "",
                    "name": "FullZero-aes128",
                    "key_type": "AES",
                    "key_version": 1,
                    "key_value": "00000000000000000000000000000000",
                    "is_sam": false,
                    "sam_key_number": 0,
                    "exportable": true
                }
            },
            "chaining": {
                "next_on_success": 0,
                "next_on_failure": 0
            }
        }
    )";


    // ----   -----   -----  ------

    std::vector<std::string> readerList =
        LibraryManager::getInstance()->getAvailableReaders();
    std::cout << "Available reader plug-ins (" << readerList.size() << "):" << std::endl;
    for (std::vector<std::string>::iterator it = readerList.begin();
         it != readerList.end(); ++it)
    {
        std::cout << "\t" << (*it) << std::endl;
    }

    std::vector<std::string> cardList =
        LibraryManager::getInstance()->getAvailableCards();
    std::cout << "Available card plug-ins (" << cardList.size() << "):" << std::endl;
    for (std::vector<std::string>::iterator it = cardList.begin(); it != cardList.end();
         ++it)
    {
        std::cout << "\t" << (*it) << std::endl;
    }

    // Reader configuration object to store reader provider and reader unit selection.
    std::shared_ptr<ReaderConfiguration> readerConfig(new ReaderConfiguration());

    readerConfig->setReaderProvider(
        LibraryManager::getInstance()->getReaderProvider("PCSC"));
    assert(readerConfig->getReaderProvider()->getReaderList().size() > 0);
    const ReaderList readers = readerConfig->getReaderProvider()->getReaderList();

    readerConfig->setReaderUnit(readers.at(0));
    std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;
    readerConfig->getReaderUnit()->connectToReader();

    // Force card type here if you want to
    // readerConfig->getReaderUnit()->setCardType(CT_DESFIRE_EV1);

    std::cout << "Time start : " << time(NULL) << std::endl;
    if (readerConfig->getReaderUnit()->waitInsertion(15000))
    {

        if (readerConfig->getReaderUnit()->connect())
        {
            std::cout << "Card inserted on reader \""
                      << readerConfig->getReaderUnit()->getConnectedName() << "\"."
                      << std::endl;

            std::shared_ptr<Chip> chip = readerConfig->getReaderUnit()->getSingleChip();
            std::cout << "Card type: " << chip->getCardType() << std::endl;

            // DO SOMETHING HERE
            // DO SOMETHING HERE
            // DO SOMETHING HERE

            auto service = chip->getService<JsonDumpCardService>();
            ASSERT_NE(nullptr, service);

            auto out = service->dump(card_template);
            ASSERT_EQ(service->formats_.size(), 1);
            ASSERT_EQ(service->formats_["r7Xhf9pByzRFwQk3X"]->getName(), "islog-presentation");
            ASSERT_EQ(service->formats_["r7Xhf9pByzRFwQk3X"]->getFieldList().size(), 6);

            std::cout << "Output: " << out << std::endl;
            readerConfig->getReaderUnit()->disconnect();
        }
        else
        {
            std::cout << "Error: cannot connect to the card." << std::endl;
        }

        std::cout << "Logical automatic card removal in 15 seconds..." << std::endl;

        if (!readerConfig->getReaderUnit()->waitRemoval(15000))
        {
            std::cerr << "Card removal forced." << std::endl;
        }

        std::cout << "Card removed." << std::endl;
    }
    else
    {
        std::cout << "No card inserted." << std::endl;
    }
}
