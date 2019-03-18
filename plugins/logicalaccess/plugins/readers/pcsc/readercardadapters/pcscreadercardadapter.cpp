/**
 * \file pcscreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader/card adapter.
 */

#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscdatatransport.hpp>

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
    return ISO7816ReaderCardAdapter::adaptCommand(command);
}

ByteVector PCSCReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    return ISO7816ReaderCardAdapter::adaptAnswer(answer);
}
}