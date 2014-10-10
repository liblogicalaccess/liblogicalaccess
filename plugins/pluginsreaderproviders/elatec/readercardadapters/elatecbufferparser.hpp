/**
 * \file elatecbufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Elatec Buffer Parser.
 */

#ifndef ELATECBUFFERPARSER_HPP
#define ELATECBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API ElatecBufferParser : public CircularBufferParser
    {
    public:
        ElatecBufferParser() {};

        virtual ~ElatecBufferParser() {};

        virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
    };
}

#endif /* ELATECBUFFERPARSER_HPP */