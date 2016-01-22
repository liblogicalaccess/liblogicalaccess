//
// Created by xaqq on 6/24/15.
//

#ifndef LIBLOGICALACCESS_LLA_FWD_HPP
#define LIBLOGICALACCESS_LLA_FWD_HPP

#include <cstdint>
#include <vector>
#include <memory>
#include "logicalaccess_api.hpp"

namespace logicalaccess
{
    class CardsFormatComposite;

    class Chip;
    using ChipPtr = std::shared_ptr<Chip>;

    class DataTransport;
    using DataTransportPtr = std::shared_ptr<DataTransport>;

    class LCDDisplay;

    class LEDBuzzerDisplay;

    class ReaderCardAdapter;

    class ReaderProvider;   
    using ReaderProviderPtr = std::shared_ptr<ReaderProvider>;
    
    class ReaderUnit;
    using ReaderUnitPtr = std::shared_ptr<ReaderUnit>;

    //class PCSCReaderUnit;
    //using PCSCReaderUnit = std::shared_ptr<PCSCReaderUnit>;

    class ReaderUnitConfiguration;

    class LocationNode;

    class Location;

    class Commands;

    class Profile;

    class CardService;

    class Format;

    class ResultChecker;

    class Key;
    using KeyPtr = std::shared_ptr<Key>;

    class TripleDESKey;
    using TripleDESKeyPtr = std::shared_ptr<TripleDESKey>;

    class KeyStorage;
    using KeyStoragePtr = std::shared_ptr<KeyStorage>;

    class ReaderMemoryKeyStorage;
    using ReaderMemoryKeyStoragePtr = std::shared_ptr<ReaderMemoryKeyStorage>;

    class ReaderService;
    using ReaderServicePtr = std::shared_ptr<ReaderService>;
}

using ByteVector = std::vector<uint8_t>;

#endif //LIBLOGICALACCESS_LLA_FWD_HPP
