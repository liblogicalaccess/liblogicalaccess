/**
 * \file axesstmc13bufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief AxessTMC13 Buffer Parser. 
 */

#ifndef AXESSTMC13BUFFERPARSER_HPP
#define AXESSTMC13BUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API AxessTMC13BufferParser : public CircularBufferParser
	{
	public:
		AxessTMC13BufferParser() {};

		virtual ~AxessTMC13BufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};

}

#endif /* AXESSTMC13BUFFERPARSER_HPP */

 
