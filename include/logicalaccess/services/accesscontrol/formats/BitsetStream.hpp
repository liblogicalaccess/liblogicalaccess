
#ifndef LOGICALACCESS_BITSETSTREAM_HPP
#define LOGICALACCESS_BITSETSTREAM_HPP

#include <vector>
#include <stdint.h>
#include <iostream>

#include <logicalaccess/logicalaccess_api.hpp>
#include "logicalaccess/lla_fwd.hpp"

namespace logicalaccess
{
	class LIBLOGICALACCESS_API BitsetStream
	{
	public:

		BitsetStream();

                BitsetStream(unsigned char data, unsigned int byteSize);

		BitsetStream(unsigned long long bitsSize);

		~BitsetStream();

		void append(unsigned char data);

		void append(unsigned char data, unsigned int readPosStart);

		void append(unsigned char data, unsigned int readPosStart, unsigned int readLength);

                void concat(const BitsetStream& data);

		void concat(const ByteVector& data);

		void concat(const ByteVector& data, unsigned int readPosStart);

		void concat(const ByteVector& data, unsigned int readPosStart, unsigned int readLength);

		void writeAt(unsigned int pos, unsigned char data, unsigned int readPosStart = 0, unsigned int readLength = 8);

		void writeAt(unsigned int pos, ByteVector const& data, unsigned int readPosStart, unsigned int readLength);

		void insert(unsigned int pos, unsigned char data, unsigned int readPosStart = 0, unsigned int readLength = 8);

		void insert(unsigned int pos, ByteVector const& data, unsigned int readPosStart, unsigned int readLength);

		ByteVector getData() const;

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

		ByteVector stream;

		unsigned int _pos;
	};
}

#endif // LOGICALACCESS_BITSETSTREAM_HPP