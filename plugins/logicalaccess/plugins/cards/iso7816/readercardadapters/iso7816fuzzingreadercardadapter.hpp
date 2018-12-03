/**
* \file iso7816fuzzingreadercardadapter.hpp
* \brief Default ISO7816 Fuzzing reader/card adapter.
*/

#ifndef LOGICALACCESS_ISO7816FUZZINGREADERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816FUZZINGREADERCARDADAPTER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>

#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <functional>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
* \brief A default ISO7816 Fuzzing reader/card adapter class.
*/
class LLA_CARDS_ISO7816_API ISO7816FuzzingReaderCardAdapter
    : public ISO7816ReaderCardAdapter
{
  public:
    ISO7816FuzzingReaderCardAdapter();

    ByteVector sendCommand(const ByteVector &command, long timeout = -1) override;

  private:
    static int index;
    int currentIndex;
};
}

#endif /* LOGICALACCESS_ISO7816FUZZINGREADERCARDADAPTER_HPP */