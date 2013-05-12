/**
 * \file iso7816readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default ISO7816 reader/card adapter. 
 */

#ifndef LOGICALACCESS_ISO7816READERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816READERCARDADAPTER_HPP

#include "logicalaccess/cards/readercardadapter.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
	/**
	 * \brief A default ISO7816 reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API ISO7816ReaderCardAdapter : public ReaderCardAdapter
	{
		public:
					
			/**
			 * \brief Send an APDU command to the reader.
			 */
			virtual void sendAPDUCommand(const std::vector<unsigned char>& command, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char le, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, const unsigned char* data, size_t datalen, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader without result.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen, unsigned char le);

			/**
			 * \brief Send an APDU command to the reader without result.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, const unsigned char* data, size_t datalen);

			/**
			 * \brief Send an APDU command to the reader without data and result.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le);

			/**
			 * \brief Send an APDU command to the reader without data.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char lc, unsigned char le, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader without data.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char le, unsigned char* result, size_t* resultlen);

			/**
			 * \brief Send an APDU command to the reader without data.
			 */
			virtual void sendAPDUCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char* result, size_t* resultlen);
			

		protected:
			
			
	};

}

#endif /* LOGICALACCESS_ISO7816READERCARDADAPTER_HPP */
