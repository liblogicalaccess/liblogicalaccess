//
// Created by xaqq on 12/4/15.
//

#include <logicalaccess/bufferhelper.hpp>
#include "atrparser.hpp"

namespace logicalaccess
{

ATRParser::ATRParser(const std::vector<uint8_t> &atr)
    : atr_(atr)
{
}


std::string ATRParser::guessCardType(const std::vector<uint8_t> &atr)
{
    ATRParser parser(atr);

    return parser.parse();
}

std::string ATRParser::guessCardType(uint8_t *atr, size_t atrlen)
{
    return guessCardType(std::vector<uint8_t>(atr, atr + atrlen));
}

std::string ATRParser::guessCardType(const std::string &atr_str)
{
    return guessCardType(BufferHelper::fromHexString(atr_str));
}

std::string ATRParser::parse()
{
    return "";
}

}