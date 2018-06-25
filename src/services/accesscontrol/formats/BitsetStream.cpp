/**
* \file		BitsetStream.cpp
* \author	Quentin S
* \brief	BitsetStream to manipulate bits easily.
*/

#include <logicalaccess/services/accesscontrol/formats/BitsetStream.hpp>

namespace logicalaccess
{
BitsetStream::BitsetStream()
    : _pos(0)
{
}

BitsetStream::BitsetStream(unsigned char data, unsigned int byteSize)
    : _pos(0)
{
    for (unsigned long long i = 0; i < byteSize; ++i)
    {
        append(data);
    }
}

BitsetStream::BitsetStream(unsigned long long bitsSize)
    : _pos(0)
{
    for (unsigned long long i = 0; i < bitsSize; ++i)
    {
        append(0x00, 0, 1);
    }
}

BitsetStream::~BitsetStream()
{
}

void BitsetStream::append(unsigned char data)
{
    append(data, 0);
}

void BitsetStream::append(unsigned char data, unsigned int readPosStart)
{
    append(data, readPosStart, 8 - readPosStart);
}

void BitsetStream::append(unsigned char data, unsigned int readPosStart,
                          unsigned int readLength)
{
    if (readPosStart > 8 || readPosStart + readLength > 8)
        throw std::out_of_range("Invalid reading size.");
    if (readLength == 0)
        return;

    unsigned int block  = _pos / 8;
    unsigned int offset = _pos % 8;


    if (block >= stream.size())
        stream.push_back(0);

    stream[block] |= (data >> (8 - (readPosStart + readLength))
                                  << ((8 - (readPosStart + readLength)) + readPosStart) &
                      0xFF) >>
                     offset;
    unsigned char newData =
        data >> (8 - (readPosStart + readLength))
                    << (8 - (readPosStart + readLength) + readPosStart) << (8 - offset);
    if (offset + readLength > 8)
        stream.push_back(newData);

    _pos += readLength;
}

void BitsetStream::concat(const BitsetStream &data)
{
    concat(data.getData(), 0, data.getBitSize());
}

void BitsetStream::concat(const ByteVector &data)
{
    concat(data, 0);
}

void BitsetStream::concat(const ByteVector &data, unsigned int readPosStart)
{
    concat(data, readPosStart, static_cast<unsigned int>(data.size()) * 8 - readPosStart);
}

void BitsetStream::concat(const ByteVector &data, unsigned int readPosStart,
                          unsigned int readLength)
{
    if (data.empty() && readLength != 0)
        throw std::invalid_argument("Data vector is empty.");
    if (readPosStart > data.size() * 8 || readLength + readPosStart > data.size() * 8)
        throw std::out_of_range("Invalid reading size.");

    unsigned int offset   = readPosStart % 8;
    unsigned int readBits = 0;
    unsigned int i        = readPosStart / 8;

    while (readBits < readLength)
    {
        if (offset + (readLength - readBits) > 8)
        {
            unsigned char length =
                (readLength - readBits) < 8 ? (readLength - readBits) : 8;
            append((data[i] << offset) | (data[i + 1] >> (8 - offset)), 0, length);
            readBits += length;
            i++;
        }
        else
        {
            append(data[i], offset, readLength - readBits);
            readBits += (readLength - readBits);
        }
    }
}

void BitsetStream::writeAt(unsigned int pos, unsigned char data,
                           unsigned int readPosStart, unsigned int readLength)
{
    if (readPosStart >= 8 || readPosStart + readLength > 8)
        throw std::out_of_range("Invalid reading size.");
    if (pos > _pos)
        throw std::out_of_range("Invalid start position.");
    if (pos >= _pos)
        throw std::out_of_range("Cannot write at this position.");

    unsigned int block  = pos / 8;
    unsigned int offset = pos % 8;

    if ((8 - offset) >= readLength)
    {
        unsigned char save = stream[block] >> (8 - offset) << (8 - offset);
        stream[block] = (((stream[block] << offset << readLength & 0xFF) >> readLength) |
                         (data << readPosStart & 0xFF)) >>
                            offset |
                        save;
    }
    else
    {
        stream[block] = stream[block] >> (8 - pos % 8) << (8 - pos % 8);
        stream[block] |= ((data >> (8 - (readPosStart + readLength)))
                              << ((8 - (readPosStart + readLength)) + readPosStart) &
                          0xFF) >>
                         offset;
        block++;
        stream[block] = (stream[block] << (pos % 8) & 0xFF) >> (pos % 8);
        stream[block] |= ((data >> (8 - (readPosStart + readLength)))
                              << ((8 - (readPosStart + readLength)) + readPosStart) &
                          0xFF)
                         << (8 - offset);
    }
}

void BitsetStream::writeAt(unsigned int pos, ByteVector const &data,
                           unsigned int readPosStart, unsigned int readLength)
{
    if (data.empty() && readLength != 0)
        throw std::invalid_argument("Data vector is empty.");
    if (readPosStart > (data.size() * 8) - 1 ||
        readPosStart + readLength > (data.size() * 8))
        throw std::out_of_range("Invalid reading size.");
    if (pos > _pos)
        throw std::out_of_range("Invalid start position.");

    unsigned int offset   = readPosStart % 8;
    unsigned int readBits = 0;
    unsigned int i        = readPosStart / 8;

    while (readBits < readLength)
    {
        if (offset + (readLength - readBits) > 8)
        {
            const unsigned char length =
                (readLength - readBits) < 8 ? (readLength - readBits) : 8;
            const unsigned char newBlock =
                (data[i] << offset & 0xFF) + (data[i + 1] >> (8 - offset));
            writeAt(pos + readBits, newBlock, 0, length);
            readBits += length;
            ++i;
        }
        else
        {
            writeAt(pos + readBits, data[i], offset, readLength - readBits);
            readBits += (readLength - readBits);
        }
    }
}

void BitsetStream::insert(unsigned int pos, unsigned char data, unsigned int readPosStart,
                          unsigned int readLength)
{
    if (readPosStart > 8 || readPosStart + readLength > 8)
        throw std::out_of_range("Invalid reading size.");
    if (pos > _pos)
        throw std::out_of_range("Invalid start position.");

    if (pos == _pos)
    {
        append(data, readPosStart, readLength);
    }
    else
    {
        unsigned int last  = static_cast<unsigned int>(stream.size()) - 1;
        unsigned int block = pos / 8;
        if (_pos % 8 < readLength)
        {
            stream.push_back(0);
            stream[last + 1] = stream[last] << (8 - readLength) & 0xFF;
        }

        for (unsigned int i = last; i != block; i--)
        {
            unsigned int a = stream[i] >> readLength;
            unsigned int b = stream[i - 1] << (8 - readLength);
            stream[i]      = a + b;
        }

        if (_pos % 8 > readLength)
        {
            unsigned char save = (stream[block] >> (8 - pos % 8) & 0xFF) << (8 - pos % 8);
            stream[block] =
                ((stream[block] << (pos % 8) & 0xFF) >> (pos % 8) >> readLength & 0xFF) |
                save;
        }
        writeAt(pos, data, readPosStart, readLength);
        _pos += readLength;
    }
}

void BitsetStream::insert(unsigned int pos, ByteVector const &data,
                          unsigned int readPosStart, unsigned int readLength)
{
    if (data.empty())
        throw std::invalid_argument("Data vector is empty.");
    if (readPosStart >= data.size() * 8 || readPosStart + readLength > data.size() * 8)
        throw std::out_of_range("Invalid reading size.");
    if (pos > _pos)
        throw std::out_of_range("Invalid start position.");

    unsigned int i = readPosStart / 8;
    while ((int)readLength > 0)
    {
        insert(pos, data[i], readPosStart % 8,
               (readLength + readPosStart > 8) ? 8 - readPosStart % 8 : readLength);
        pos += (readLength + readPosStart > 8) ? 8 - readPosStart % 8 : readLength;
        readPosStart = 0;
        readLength -= (8 - readLength % 8);
        i++;
    }
}

ByteVector BitsetStream::getData() const
{
    return stream;
}

unsigned int BitsetStream::getByteSize() const
{
    return static_cast<unsigned int>(stream.size());
}

unsigned int BitsetStream::getBitSize() const
{
    if (_pos == 0)
        return static_cast<unsigned int>(stream.size()) * 8;
    return _pos;
}

std::string BitsetStream::toString() const
{
    std::string tmp("");
    unsigned char value = 0;

    for (unsigned int i = 0; i / 8 < stream.size() && i < _pos; i++)
    {
        value = stream[i / 8] >> (7 - i % 8) & 0x01;
        if (value == 1)
            tmp += '1';
        else
            tmp += '0';
    }
    return tmp;
}

std::string BitsetStream::toString(size_t begin, size_t end) const
{
    std::string str;
    unsigned char tmp = 0;

    for (size_t i = begin; i <= end && i / 8 < stream.size() && i < _pos; i++)
    {
        tmp = stream[i / 8] >> (7 - i % 8) & 0x01;
        if (tmp == 1)
            str.append("1");
        else
            str.append("0");
    }
    return str;
}

unsigned long BitsetStream::toULong() const
{
    if (stream.size() > 5)
        throw std::overflow_error("Base value overflows the size of an unsigned long.");

    unsigned long tmp = 0;

    for (unsigned int i = 0; i < stream.size(); i++)
    {
        tmp |= stream[i] << (i * 8);
    }
    return tmp;
}

unsigned long long BitsetStream::toULLong() const
{
    if (stream.size() > 8)
        throw std::overflow_error("Base value overflows the size of an unsigned long.");

    unsigned long long tmp = 0;

    for (unsigned int i = 0; i < stream.size(); i++)
    {
        tmp |= stream[i] << (i * 8);
    }
    return tmp;
}

bool BitsetStream::test(size_t index) const
{
    if (index >= _pos)
        throw std::out_of_range("Index is out of bounds.");
    if (((stream[index / 8] >> (7 - index % 8)) & 0x01) == 1)
        return true;
    return false;
}

bool BitsetStream::none() const
{
    for (unsigned int i = 0; i < stream.size(); i++)
        if (stream[i] != 0)
            return false;
    return true;
}

bool BitsetStream::any() const
{
    for (unsigned int i = 0; i < stream.size(); i++)
        if (stream[i] != 0)
            return true;
    return false;
}

bool BitsetStream::all() const
{
    for (unsigned int i = 0; i < stream.size(); i++)
        if (stream[i] != 1)
            return false;
    return true;
}

void BitsetStream::clear()
{
    stream.clear();
    _pos = 0;
}

void BitsetStream::print() const
{
    std::cout << toString().c_str() << std::endl;
    for (ByteVector::const_iterator it = stream.begin(); it != stream.end(); ++it)
    {
        std::cout << std::hex << int(*it) << " - ";
    }
    std::cout << std::endl;
}
}