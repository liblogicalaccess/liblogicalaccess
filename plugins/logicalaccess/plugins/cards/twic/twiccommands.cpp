/**
 * \file twiccommands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic commands.
 */

#include <logicalaccess/plugins/cards/twic/twiccommands.hpp>
#include <cstring>

namespace logicalaccess
{
size_t TwicCommands::getMinimumBytesRepresentation(size_t value)
{
    size_t encodedlength = 0;

    // Will works only in LITTLE ENDIAN representation.
    for (size_t i = sizeof(value) - 1; i > 0 && encodedlength == 0; --i)
    {
        if (reinterpret_cast<unsigned char *>(&value)[i] != 0x00)
        {
            encodedlength = i;
        }
    }

    return (encodedlength + 1);
}

size_t TwicCommands::getValueFromBytes(unsigned char *data, size_t datalength)
{
    size_t value = 0;
    for (size_t i = 0, n = datalength - 1; i < datalength; i++, n--)
    {
        value |= static_cast<unsigned char>(data[i] << (n * 8));
    }
    return value;
}

size_t TwicCommands::getMaximumDataObjectLength(int64_t dataObject)
{
    size_t maxlen = 0;

    switch (dataObject)
    {
    case 0x5FC104: maxlen = 57; break;

    case 0xDFC101: maxlen = 40; break;

    case 0x5FC102: maxlen = 2461; break;

    case 0xDFC103: maxlen = 2504; break;

    case 0xDFC10F: maxlen = 920; break;
    default:;
    }

    return maxlen;
}

size_t TwicCommands::getMaximumTagLength(int64_t dataObject, unsigned char tag)
{
    size_t maxlen = 0;

    switch (dataObject)
    {
    case 0x5FC104:
        switch (tag)
        {
        case 0x30: maxlen = 25; break;

        case 0x34: maxlen = 16; break;

        case 0x35: maxlen = 8; break;

        case 0xFE: maxlen = 0; break;
        default:;
        }
        break;

    case 0xDFC101:
        switch (tag)
        {
        case 0xC0: maxlen = 32; break;

        case 0xC1: maxlen = 1; break;

        case 0xC2: maxlen = 1; break;
        default:;
        }
        break;

    case 0x5FC102:
        switch (tag)
        {
        case 0x30: maxlen = 25; break;

        case 0x34: maxlen = 16; break;

        case 0x35: maxlen = 8; break;

        case 0x3E: maxlen = 2400; break;

        case 0xFE: maxlen = 0; break;
        default:;
        }
        break;

    case 0xDFC103:
        switch (tag)
        {
        case 0xBC: maxlen = 2500; break;
        default:;
        }
        break;

    case 0xDFC10F:
        switch (tag)
        {
        case 0xBA: maxlen = 12; break;

        case 0xBB: maxlen = 900; break;

        case 0xFE: maxlen = 0; break;
        default:;
        }
        break;
    default:;
    }

    return maxlen;
}

size_t TwicCommands::getDataObjectLength(int64_t dataObject, bool withObjectLength)
{
    size_t maxlen        = getMaximumDataObjectLength(dataObject);
    size_t length        = 0;
    size_t encodedlength = getMinimumBytesRepresentation(maxlen);
    if (encodedlength > 0)
    {
        size_t datalength = encodedlength + 1;
        try
        {
            ByteVector data = getISO7816Commands()->getDataList(dataObject);
            if (data.size())
            {
                length = getValueFromBytes(&data[1], datalength - 1);
            }
        }
        catch (std::exception &)
        {
            return 0;
        }
    }

    if (withObjectLength)
    {
        length += 1 + encodedlength;
    }

    return length;
}

bool TwicCommands::getTagData(std::shared_ptr<TwicLocation> location, void *data,
                              size_t datalen, void *datatag, size_t &datataglen)
{
    bool ret              = false;
    bool processed        = false;
    unsigned char *ucdata = reinterpret_cast<unsigned char *>(data);
    size_t pos            = 0;
    while (pos < datalen && !processed)
    {
        unsigned char currentTag = ucdata[pos++];
        size_t encodedlength     = getMinimumBytesRepresentation(
            getMaximumTagLength(location->dataObject, location->tag));
        size_t currentTagLength = getValueFromBytes(&ucdata[pos], encodedlength);
        pos += encodedlength;
        if (currentTag == location->tag)
        {
            processed = true;
            if (datatag != nullptr)
            {
                if (datataglen >= currentTagLength)
                {
                    memcpy(datatag, &ucdata[pos], datataglen);
                    ret = true;
                }
            }
            else
            {
                datataglen = currentTagLength;
            }
            pos += currentTagLength;
        }
    }

    return ret;
}
}