/**
* \file		BitsetStream.cpp
* \author	Quentin S
* \brief	BitsetStream to manipulate bits easily.
*/

#include "logicalaccess/services/accesscontrol/formats/BitsetStream.hpp"

namespace logicalaccess
{
	BitsetStream::BitsetStream()
	{
		this->pos = 0;
	}

	BitsetStream::BitsetStream(unsigned char data, unsigned int size)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			this->append(data);
		}
		this->pos = 0;
	}

	BitsetStream::~BitsetStream()
	{
	}

	void BitsetStream::append(unsigned char data)
	{
		this->append(data, 0);
	}

	void BitsetStream::append(unsigned char data, unsigned int readPosStart)
	{
		this->append(data, readPosStart, 8 - readPosStart);
	}

	void BitsetStream::append(unsigned char data, unsigned int readPosStart, unsigned int readLength)
	{
		if (readPosStart > 8 || readPosStart + readLength > 8)
			throw std::out_of_range("Invalid reading size.");
		if (readLength == 0)
			return;

		unsigned int block = this->pos / 8;
		unsigned int offset = this->pos % 8;

		if ((8 - offset) < readLength)
		{
			this->stream[block] |= data >> (8 - (readPosStart + readLength)) << ((8 - (readPosStart + readLength)) + readPosStart) >> offset;
			unsigned char newData = 0;
			newData |= data >> (8 - (readPosStart + readLength)) << (8 - (readPosStart + readLength)) << (8 - offset);
			if (newData != 0)
				this->stream.push_back(newData);
		}
		else
		{
			if (block >= this->stream.size())
				this->stream.push_back(0);
			this->stream[block] |= data >> (8 - (readPosStart + readLength)) << ((8 - (readPosStart + readLength)) + readPosStart) >> offset;
		}
		this->pos += readLength;
	}

	void BitsetStream::concat(const std::vector<unsigned char>& data)
	{
		this->concat(data, 0);
	}

	void BitsetStream::concat(const std::vector<unsigned char>& data, unsigned int readPosStart)
	{
		this->concat(data, readPosStart, data.size() * 8 - readPosStart);
	}

	void BitsetStream::concat(const std::vector<unsigned char>& data, unsigned int readPosStart, unsigned int readLength)
	{
		if (data.empty())
			throw std::invalid_argument("Data vector is empty.");
		if (readPosStart > data.size() * 8 || readLength + readPosStart > data.size() * 8)
			throw std::out_of_range("Invalid reading size.");

		unsigned int offset = readPosStart % 8;
		unsigned char newBlock;
		unsigned int readBits = 0;
		unsigned int i = readPosStart / 8;

		while (readBits < readLength)
		{
			if (readLength - readBits > 8)
			{
				newBlock = data[i] << offset;
				newBlock = (data[i + 1] >> (8 - offset));
				newBlock = (data[i] << offset) + (data[i + 1] >> (8 - offset));
				this->append(newBlock);
				readBits += 8;
				i++;
			}
			else
			{
				this->append(data[i], offset, readLength - readBits);
				readBits += (readLength - readBits);
			}
		}
	}

	void BitsetStream::writeAt(unsigned int pos, unsigned char data, unsigned int readPosStart, unsigned int readLength)
	{
		if (readPosStart >= 8 || readPosStart + readLength > 8)
			throw std::out_of_range("Invalid reading size.");
		if (pos > this->pos)
			throw std::out_of_range("Invalid start position.");
		if (pos >= this->pos)
			throw std::out_of_range("Cannot write at this position.");

		unsigned int block = pos / 8;
		unsigned int offset = pos % 8;

		if ((8 - offset) >= readLength)
		{
			unsigned char save = this->stream[block] >> (8 - offset) << (8 - offset);
			this->stream[block] = (((this->stream[block] << offset << readLength & 0xFF) >> readLength) | (data << readPosStart)) >> offset | save;
		}
		else
		{
			this->stream[block] = this->stream[block] >> (8 - pos % 8) << (8 - pos % 8);
			this->stream[block] |= data >> (8 - (readPosStart + readLength)) << ((8 - (readPosStart + readLength)) + readPosStart) >> offset;
			block++;
			this->stream[block] = (this->stream[block] << (pos % 8) & 0xFF) >> (pos % 8);
			this->stream[block] |= data >> (8 - (readPosStart + readLength)) << ((8 - (readPosStart + readLength)) + readPosStart) << (8 - offset);
		}
	}

	void BitsetStream::writeAt(unsigned int pos, std::vector<unsigned char> const& data, unsigned int readPosStart, unsigned int readLength)
	{
		if (data.empty())
			throw std::invalid_argument("Data vector is empty.");
		if (readPosStart > (data.size() * 8) - 1 || readPosStart + readLength > (data.size() * 8))
			throw std::out_of_range("Invalid reading size.");
		if (pos > this->pos)
			throw std::out_of_range("Invalid start position.");

		unsigned int offset = readPosStart % 8;
		unsigned char newBlock;
		unsigned int readBits = 0;
		unsigned int i = readPosStart / 8;

		while (readBits < readLength)
		{
			if (readLength - readBits > 8)
			{
				newBlock = data[i] << offset;
				newBlock = (data[i + 1] >> (8 - offset));
				newBlock = (data[i] << offset) + (data[i + 1] >> (8 - offset));
				this->writeAt(pos, newBlock, 0, 8);
				readBits += 8;
				i++;
				pos = readBits;
			}
			else
			{
				this->writeAt(pos, data[i], offset, readLength - readBits);
				readBits += (readLength - readBits);
			}
		}
	}

	void BitsetStream::insert(unsigned int pos, unsigned char data, unsigned int readPosStart, unsigned int readLength)
	{
		if (readPosStart > 8 || readPosStart + readLength > 8)
			throw std::out_of_range("Invalid reading size.");
		if (pos > this->pos)
			throw std::out_of_range("Invalid start position.");

		if (pos == this->pos)
		{
			this->append(data, readPosStart, readLength);
		}
		else
		{
			unsigned int last = this->stream.size() - 1;
			unsigned int block = pos / 8;
			if (this->pos % 8 < readLength)
			{
				this->stream.push_back(0);
				this->stream[last + 1] = this->stream[last] << (8 - readLength) & 0xFF;
			}

			for (unsigned int i = last; i != block; i--)
			{
				unsigned int a = this->stream[i] >> readLength;
				unsigned int b = this->stream[i - 1] << (8 - readLength);
				this->stream[i] = a + b;
			}

			if (this->pos % 8 > readLength)
			{
				unsigned char save = (this->stream[block] >> (8 - pos % 8) & 0xFF) << (8 - pos % 8);
				this->stream[block] = (this->stream[block] << (pos % 8) & 0xFF) >> (pos % 8) >> readLength & 0xFF | save;
			}
			this->writeAt(pos, data, readPosStart, readLength);
			this->pos += readLength;
		}
	}

	void BitsetStream::insert(unsigned int pos, std::vector<unsigned char> const& data, unsigned int readPosStart, unsigned int readLength)
	{
		if (data.empty())
			throw std::invalid_argument("Data vector is empty.");
		if (readPosStart >= data.size() * 8 || readPosStart + readLength > data.size() * 8)
			throw std::out_of_range("Invalid reading size.");
		if (pos > this->pos)
			throw std::out_of_range("Invalid start position.");

		unsigned int i = readPosStart / 8;
		while ((int)readLength > 0)
		{
			insert(pos, data[i], readPosStart % 8, (readLength + readPosStart > 8) ? 8 - readPosStart % 8 : readLength);
			pos += (readLength + readPosStart > 8) ? 8 - readPosStart % 8 : readLength;
			readPosStart = 0;
			readLength -= (8 - readLength % 8);
			i++;
		}
	}

	std::vector<uint8_t> BitsetStream::getData() const
	{
		return this->stream;
	}

	unsigned int BitsetStream::getByteSize() const
	{
		return this->stream.size();
	}

	unsigned int BitsetStream::getBitSize() const
	{
		if (this->pos == 0)
			return this->stream.size() * 8;
		return this->pos;
	}

	std::string BitsetStream::toString() const
	{
		std::string tmp("");
		unsigned char value = 0;

		for (unsigned int i = 0; i / 8 < this->stream.size() && i < this->pos; i++)
		{
			value = this->stream[i / 8] >> (7 - i % 8) & 0x01;
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

		for (size_t i = begin; i <= end && i / 8 < this->stream.size() && i < this->pos; i++)
		{
			tmp = this->stream[i / 8] >> (7 - i % 8) & 0x01;
			if (tmp == 1)
				str.append("1");
			else
				str.append("0");
		}
		return str;
	}

	unsigned long BitsetStream::toULong() const
	{
		if (this->stream.size() > 5)
			throw std::overflow_error("Base value overflows the size of an unsigned long.");

		unsigned long tmp = 0;

		for (unsigned int i = 0; i < this->stream.size(); i++)
		{
			tmp |= this->stream[i] << (i * 8);
		}
		return tmp;
	}

	unsigned long long BitsetStream::toULLong() const
	{
		if (this->stream.size() > 8)
			throw std::overflow_error("Base value overflows the size of an unsigned long.");

		unsigned long long tmp = 0;

		for (unsigned int i = 0; i < this->stream.size(); i++)
		{
			tmp |= this->stream[i] << (i * 8);
		}
		return tmp;
	}

	bool BitsetStream::test(size_t index) const
	{
		if (index >= this->pos)
			throw std::out_of_range("Index is out of bounds.");
		unsigned char tmp = this->stream[index / 8] >> (7 - index % 8);
		if (((this->stream[index / 8] >> (7 - index % 8)) & 0x01) == 1)
			return true;
		return false;
	}

	bool BitsetStream::none() const
	{
		for (unsigned int i = 0; i < this->stream.size(); i++)
			if (this->stream[i] != 0)
				return false;
		return true;
	}

	bool BitsetStream::any() const
	{
		for (unsigned int i = 0; i < this->stream.size(); i++)
			if (this->stream[i] != 0)
				return true;
		return false;
	}

	bool BitsetStream::all() const
	{
		for (unsigned int i = 0; i < this->stream.size(); i++)
			if (this->stream[i] != 1)
				return false;
		return true;
	}

	void BitsetStream::clear()
	{
		this->stream.clear();
		this->pos = 0;
	}

	void BitsetStream::print() const
	{
		std::cout << this->toString().c_str() << std::endl;
		for (std::vector<unsigned char>::const_iterator it = this->stream.begin(); it != this->stream.end(); it++)
		{
			std::cout << std::hex << int(*it) << " - ";
		}
		std::cout << std::endl;
	}
}