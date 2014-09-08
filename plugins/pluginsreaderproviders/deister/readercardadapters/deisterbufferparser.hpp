/**
 * \file deisterbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Deister Buffer Parser. 
 */

#ifndef DEISTERBUFFERPARSER_HPP
#define DEISTERBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API DeisterBufferParser : public CircularBufferParser
	{
	public:
		DeisterBufferParser() {};

		virtual ~DeisterBufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};

}

#endif /* DEISTERBUFFERPARSER_HPP */

 
