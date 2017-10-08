/**
 * \file axesstmc13readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC 13Mhz reader/card adapter.
 */

#include "axesstmc13readercardadapter.hpp"

namespace logicalaccess
{
    const unsigned char AxessTMC13ReaderCardAdapter::START0 = 'N';
    const unsigned char AxessTMC13ReaderCardAdapter::START1 = 'F';
    const unsigned char AxessTMC13ReaderCardAdapter::CR = 0x0d;

    AxessTMC13ReaderCardAdapter::AxessTMC13ReaderCardAdapter()
        : ReaderCardAdapter()
    {
    }

    AxessTMC13ReaderCardAdapter::~AxessTMC13ReaderCardAdapter()
    {
    }

    ByteVector AxessTMC13ReaderCardAdapter::adaptCommand(const ByteVector& command)
    {
        return command;
    }

    ByteVector AxessTMC13ReaderCardAdapter::adaptAnswer(const ByteVector& answer)
    {
        EXCEPTION_ASSERT_WITH_LOG(answer.size() > 0, std::invalid_argument, "A valid command buffer size must be at least 1 bytes long");
        EXCEPTION_ASSERT_WITH_LOG(answer[answer.size() - 1] == CR, std::invalid_argument, "The supplied command buffer is not valid");

        return ByteVector(answer.begin(), answer.end() - 1);
    }
}