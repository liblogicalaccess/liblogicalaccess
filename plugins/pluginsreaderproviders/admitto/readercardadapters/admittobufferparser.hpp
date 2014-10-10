/**
 * \file admittobufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Admitto Buffer Parser.
 */

#ifndef ADMITTOBUFFERPARSER_HPP
#define ADMITTOBUFFERPARSER_HPP

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API AdmittoBufferParser : public CircularBufferParser
    {
    public:
        AdmittoBufferParser() {};

        virtual ~AdmittoBufferParser() {};

        virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);
    };
}

#endif /* ADMITTOBUFFERPARSER_HPP */