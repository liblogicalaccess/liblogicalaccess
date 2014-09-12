/**
 * \file promagbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Promag Buffer Parser. 
 */

#ifndef PROMAGBUFFERPARSER_HPP
#define PROMAGBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	class LIBLOGICALACCESS_API PromagBufferParser : public CircularBufferParser
	{
	public:
		PromagBufferParser() {};

		virtual ~PromagBufferParser() {};

		virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
	};

}

#endif /* PROMAGBUFFERPARSER_HPP */

 
