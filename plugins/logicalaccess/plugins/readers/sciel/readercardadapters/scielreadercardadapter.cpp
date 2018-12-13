/**
 * \file scielreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SCIEL reader/card adapter.
 */

#include <logicalaccess/plugins/readers/sciel/readercardadapters/scielreadercardadapter.hpp>
#include <logicalaccess/plugins/readers/sciel/readercardadapters/scielserialportdatatransport.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
const unsigned char SCIELReaderCardAdapter::STX = 0x5b; // [
const unsigned char SCIELReaderCardAdapter::ETX = 0x5d; // ]

SCIELReaderCardAdapter::SCIELReaderCardAdapter()
    : ReaderCardAdapter()
{
}

SCIELReaderCardAdapter::~SCIELReaderCardAdapter()
{
}

ByteVector SCIELReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    ByteVector cmd;
    // Ignore empty command
    if (command.size() > 0)
    {
        cmd.push_back(STX);
        cmd.insert(cmd.end(), command.begin(), command.end());
        cmd.push_back(ETX);
    }

    return cmd;
}

ByteVector SCIELReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    EXCEPTION_ASSERT_WITH_LOG(answer.size() >= 2, LibLogicalAccessException,
                              "A valid buffer size must be at least 2 bytes long");
    EXCEPTION_ASSERT_WITH_LOG(answer[0] == STX, LibLogicalAccessException,
                              "The supplied buffer is not valid");
    EXCEPTION_ASSERT_WITH_LOG(answer[answer.size() - 1] == ETX, LibLogicalAccessException,
                              "Missing end of message");

    ByteVector data(answer.begin() + 1, answer.end() - 1);

    LOG(LogLevel::COMS) << "Returning processed data " << BufferHelper::getHex(data)
                        << "...";
    return data;
}

std::list<ByteVector>
SCIELReaderCardAdapter::receiveTagsListCommand(const ByteVector &command,
                                               long int timeout)
{
    LOG(LogLevel::COMS)
        << "Beginning receiving tag(s) identifiers whole list... Timeout configured {"
        << timeout << "}";

    std::list<ByteVector> tagsList;

    ByteVector tag = sendCommand(command, timeout);
    bool done      = false;
    while (!done && tag.size() > 0)
    {
        if (tag.size() == 2)
        {
            LOG(LogLevel::COMS) << "Whole list has been received successfully!";
            done = true;
        }
        else
        {
            tagsList.push_back(tag);

            std::shared_ptr<ScielSerialPortDataTransport> sdt =
                std::dynamic_pointer_cast<ScielSerialPortDataTransport>(getDataTransport());
            if (sdt)
            {
                ByteVector tagbuf = sdt->checkValideBufferAvailable();
                if (tagbuf.size() > 0)
                {
                    tag = adaptAnswer(tagbuf);
                }
                else
                {
                    tag = ByteVector();
                }
            }
            else
            {
                tag = ByteVector();
            }
        }
    }

    return tagsList;
}
}