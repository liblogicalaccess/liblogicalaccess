//
// Created by xaqq on 7/31/15.
//

#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcsc_ctl_datatransport.hpp>


// Explanation here
// http://tech.springcard.com/tags/scard_ctl_code/
#ifdef WIN32
#define IOCTL_CCID_ESCAPE SCARD_CTL_CODE(3500)
#else
#include <reader.h>
#define IOCTL_CCID_ESCAPE SCARD_CTL_CODE(3500)
#endif

void logicalaccess::PCSCControlDataTransport::send(const ByteVector &data)
{
    LLA_LOG_CTX("PCSC Control DataTransport");

    d_response.clear();

    EXCEPTION_ASSERT_WITH_LOG(getPCSCReaderUnit(), LibLogicalAccessException,
                              "The PCSC reader unit object"
                              "is null. We cannot send.");
    if (data.size() > 0)
    {
        std::array<uint8_t, 255> returnedData;
        ULONG ulNoOfDataReceived;

        LOG(LogLevel::COMS) << "APDU (control) command: " << BufferHelper::getHex(data);

        unsigned int errorFlag = SCardControl(
            getPCSCReaderUnit()->getHandle(), IOCTL_CCID_ESCAPE, &data[0], data.size(),
            &returnedData[0], returnedData.size(), &ulNoOfDataReceived);
        CheckCardError(errorFlag);
        d_response =
            ByteVector(returnedData.begin(), returnedData.begin() + ulNoOfDataReceived);
    }
}
