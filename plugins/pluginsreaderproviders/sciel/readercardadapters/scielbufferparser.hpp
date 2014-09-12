/**
 * \file scielbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Sciel Buffer Parser. 
 */

#ifndef SCIELBUFFERPARSER_HPP
#define SCIELBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API ScielBufferParser : public CircularBufferParser
	{
	public:
		ScielBufferParser() {};

		virtual ~ScielBufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};

}

#endif /* SCIELBUFFERPARSER_HPP */

 
