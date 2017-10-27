/**
 * \file pcscreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader/card adapter.
 */

#include "pcscreadercardadapter.hpp"
#include "../pcscreaderunit.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "../pcscdatatransport.hpp"

namespace logicalaccess
{
PCSCReaderCardAdapter::PCSCReaderCardAdapter()
{
    d_dataTransport.reset(new PCSCDataTransport());
}

PCSCReaderCardAdapter::~PCSCReaderCardAdapter()
{
}

ByteVector PCSCReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    return command;
}

ByteVector PCSCReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    return answer;
}
}