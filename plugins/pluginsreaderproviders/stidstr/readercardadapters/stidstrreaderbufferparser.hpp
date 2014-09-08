/**
 * \file stidstrreaderbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief STidSTR Buffer Parser. 
 */

#ifndef STIDSTRBUFFERPARSER_HPP
#define STIDSTRBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API STidSTRBufferParser : public CircularBufferParser
	{
	public:
		STidSTRBufferParser() {};

		virtual ~STidSTRBufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};

}

#endif /* STIDSTRBUFFERPARSER_HPP */

 
