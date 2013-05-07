/**
 * \file Encoding.h
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime-dev@islog.com>
 * \brief Encoding.
 */

#ifndef LOGICALACCESS_ENCODING_H
#define LOGICALACCESS_ENCODING_H

#include <string>

using std::string;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{
	/**
	 * \brief Encoding type
	 */	
	typedef enum {
		ET_UNKNOWN = 0x00,
		ET_BCDBYTE = 0x01,
		ET_BCDNIBBLE = 0x02,
		ET_BINARY = 0x03,
		ET_BIGENDIAN = 0x04,
		ET_LITTLEENDIAN = 0x05,
		ET_NOENCODING = 0x06
	} EncodingType;


	/**
	 * \brief An encoder type.
	 */
	class LIBLOGICALACCESS_API Encoding
	{
		public:

			/**
			 * \brief Get the encoder name.
			 * \return The encoder name.
			 */
			virtual string getName() const = 0;

			/**
			 * \brief Get the encoder type.
			 * \return The encoder type.
			 */
			virtual EncodingType getType() const = 0;
	};
}

#endif /* LOGICALACCESS_ENCODING_H */
