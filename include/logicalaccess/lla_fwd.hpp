//
// Created by xaqq on 6/24/15.
//

#ifndef LIBLOGICALACCESS_LLA_FWD_HPP
#define LIBLOGICALACCESS_LLA_FWD_HPP

#include <cstdint>
#include <vector>
#include "logicalaccess_api.hpp"

namespace logicalaccess
{
    class CardsFormatComposite;

    class Chip;

    class DataTransport;

    class LCDDisplay;

    class LEDBuzzerDisplay;

    class ReaderCardAdapter;

    class ReaderProvider;

    class ReaderUnit;

    class ReaderUnitConfiguration;

    class LocationNode;

    class Location;

    class Commands;

    class Profile;

    class CardService;

    class Format;

    class ResultChecker;
}

using ByteVector = std::vector<uint8_t>;

#endif //LIBLOGICALACCESS_LLA_FWD_HPP
