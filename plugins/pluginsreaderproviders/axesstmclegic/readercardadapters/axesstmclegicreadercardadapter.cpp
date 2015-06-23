/**
 * \file axesstmclegicreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMCLegic reader/card adapter.
 */

#include <logicalaccess/logs.hpp>
#include "axesstmclegicreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{
    AxessTMCLegicReaderCardAdapter::AxessTMCLegicReaderCardAdapter()
        : ReaderCardAdapter()
    {
    }

    AxessTMCLegicReaderCardAdapter::~AxessTMCLegicReaderCardAdapter()
    {
    }

    std::vector<unsigned char> AxessTMCLegicReaderCardAdapter::adaptCommand(const std::vector<unsigned char>& command)
    {
        return command;
    }

    std::vector<unsigned char> AxessTMCLegicReaderCardAdapter::adaptAnswer(const std::vector<unsigned char>& answer)
    {
        EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 1, std::invalid_argument, "A valid command buffer size must be at least 1 bytes long");
        unsigned char msglength = answer[0];
        EXCEPTION_ASSERT_WITH_LOG(answer.size() >= static_cast<unsigned char>(1 + msglength), std::invalid_argument, "The supplied command buffer is not valid (bad messange length)");

        return std::vector<unsigned char>(answer.begin() + 1, answer.begin() + 1 + msglength);
    }
}