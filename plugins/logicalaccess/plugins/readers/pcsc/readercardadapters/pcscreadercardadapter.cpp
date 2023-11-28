/**
 * \file pcscreadercardadapter.cpp
 * \author Maxime C. <maxime@leosac.com>
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

PCSCReaderCardAdapter::~PCSCReaderCardAdapter() {}

}
