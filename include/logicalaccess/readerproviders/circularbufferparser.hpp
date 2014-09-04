/**
 * \file circularbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief A CircularBufferParser class.
 */

#ifndef CIRCULARBUFFERPARSER_HPP
#define CIRCULARBUFFERPARSER_HPP

#include <vector>
#include <boost/circular_buffer.hpp>

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#endif

namespace logicalaccess
{
	class LIBLOGICALACCESS_API CircularBufferParser
	{
	public:
		CircularBufferParser() {};
		~CircularBufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};
}

#endif /* CIRCULARBUFFERPARSER_HPP */
