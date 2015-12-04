#ifndef LIBLOGICALACCESS_ATRPARSER_HPP
#define LIBLOGICALACCESS_ATRPARSER_HPP

#include "logicalaccess/logicalaccess_api.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>

namespace logicalaccess
{
/**
 * An helper class that helps deducing the type of card
 * from its ATR.
 */
class LIBLOGICALACCESS_API ATRParser
{
      public:
        ATRParser(const std::vector<uint8_t> &atr);

        static std::string guessCardType(const std::vector<uint8_t> &atr);
        static std::string guessCardType(uint8_t *atr, size_t atrlen);
        /**
         * Guess the card from an hexadecimal string representation
         * of the ATR.
         */
        static std::string guessCardType(const std::string &atr_str);

      private:
        std::string parse();
        std::vector<uint8_t> atr_;
};

}

#endif //LIBLOGICALACCESS_ATRPARSER_HPP
