
#ifndef LOGICALACCESS_BITSETSTREAM_HPP
#define LOGICALACCESS_BITSETSTREAM_HPP

#include <vector>
#include <stdint.h>
#include <iostream>

#include <logicalaccess/logicalaccess_api.hpp>

namespace logicalaccess
{
	class LIBLOGICALACCESS_API BitsetStream
	{
	public:

		BitsetStream();

		BitsetStream(unsigned char, unsigned int);

		~BitsetStream();

		void append(unsigned char data);

		void append(unsigned char data, unsigned int readPosStart);

		void append(unsigned char data, unsigned int readPosStart, unsigned int readLength);

		void concat(const std::vector<unsigned char>& data);

		void concat(const std::vector<unsigned char>& data, unsigned int readPosStart);

		void concat(const std::vector<unsigned char>& data, unsigned int readPosStart, unsigned int readLength);

		void writeAt(unsigned int pos, unsigned char data, unsigned int readPosStart = 0, unsigned int readLength = 8);

		void writeAt(unsigned int pos, std::vector<unsigned char> const& data, unsigned int readPosStart, unsigned int readLength);

		void insert(unsigned int pos, unsigned char data, unsigned int readPosStart = 0, unsigned int readLength = 8);

		void insert(unsigned int pos, std::vector<unsigned char> const& data, unsigned int readPosStart, unsigned int readLength);

		std::vector<uint8_t> getData() const;

		unsigned int getByteSize() const;

		unsigned int getBitSize() const;

		std::string toString(size_t begin, size_t end) const;

		std::string toString() const;

		unsigned long toULong() const;

		unsigned long long toULLong() const;

		bool test(size_t index) const;

		bool none() const;

		bool any() const;

		bool all() const;

		void print() const;

		void clear();

	private:

		std::vector<uint8_t> stream;

		unsigned int pos;
	};
}

#endif // LOGICALACCESS_BITSETSTREAM_HPP