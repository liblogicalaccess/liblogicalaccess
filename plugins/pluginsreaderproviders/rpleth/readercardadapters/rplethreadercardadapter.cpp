/**
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "rplethreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "../rplethdatatransport.hpp"

#include "logicalaccess/settings.hpp"

namespace logicalaccess
{
RplethReaderCardAdapter::RplethReaderCardAdapter()
    : ReaderCardAdapter()
{
}

RplethReaderCardAdapter::~RplethReaderCardAdapter()
{
}

ByteVector RplethReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    return command;
}

ByteVector RplethReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    return answer;
}

ByteVector RplethReaderCardAdapter::sendRplethCommand(const ByteVector &data,
                                                      bool waitanswer, long timeout)
{
    if (timeout == -1)
        timeout = Settings::getInstance()->DataTransportTimeout;

    LOG(LogLevel::COMS) << "Send Rpleth Command : " << BufferHelper::getHex(data);
    ByteVector res;

    std::shared_ptr<RplethDataTransport> dt =
        std::dynamic_pointer_cast<RplethDataTransport>(getDataTransport());
    if (dt)
    {
        if (!waitanswer)
        {
            dt->sendPing();
        }
        if (adaptCommand(data).size() > 0)
        {
            dt->sendll(adaptCommand(data));
        }
        res = adaptAnswer(dt->receive(timeout));
    }
    else
    {
        LOG(LogLevel::ERRORS) << "Not a Rpleth data transport.";
    }

    return res;
}
}