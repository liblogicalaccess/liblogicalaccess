/**
 * \file gunnebobufferparser.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief Gunnebo Buffer Parser.
 */

#pragma once

#include "logicalaccess/readerproviders/circularbufferparser.hpp"

#include <string>
#include <vector>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API GunneboBufferParser : public CircularBufferParser
    {
    public:
        GunneboBufferParser();

        virtual ~GunneboBufferParser() = default;

        virtual std::vector<unsigned char> getValidBuffer(boost::circular_buffer<unsigned char>& circular_buffer);

        void setChecksum(bool checksum);

    protected:

        bool d_checksum;
    };
}

