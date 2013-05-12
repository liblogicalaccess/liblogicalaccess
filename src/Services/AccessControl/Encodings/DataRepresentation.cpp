/**
 * \file datarepresentation.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A Data Representation.
 */

#include "logicalaccess/services/accesscontrol/encodings/datarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/bigendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/littleendiandatarepresentation.hpp"
#include "logicalaccess/services/accesscontrol/encodings/nodatarepresentation.hpp"

namespace logicalaccess
{
	DataRepresentation* DataRepresentation::getByEncodingType(EncodingType type)
	{
		switch (type)
		{
		case ET_BIGENDIAN:
			return new BigEndianDataRepresentation();
			break;

		case ET_LITTLEENDIAN:
			return new LittleEndianDataRepresentation();
			break;

		case ET_NOENCODING:
			return new NoDataRepresentation();
			break;

		default:
			return NULL;
			break;
		}
	}
}

