/**
 * \file rplethreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default Rpleth reader/card adapter. 
 */

#ifndef LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP

#include "rplethreadercardadapter.hpp"
#include "readercardadapters/iso7816readercardadapter.hpp"

#include <string>
#include <vector>


namespace logicalaccess
{	
	/**
	 * \brief A default Rpleth reader/card adapter class.
	 */
	class LIBLOGICALACCESS_API ISO7816RplethReaderCardAdapter : public ISO7816ReaderCardAdapter
	{
		public:

			/**
			 * \brief Constructor.
			 */
			ISO7816RplethReaderCardAdapter();

			/**
			 * \brief Destructor.
			 */
			virtual ~ISO7816RplethReaderCardAdapter();

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
			
			boost::shared_ptr<RplethReaderCardAdapter> d_rpleth_reader_card_adapter;

			std::vector<unsigned char> handleAnswer (const std::vector<unsigned char>& answer);

			std::vector<unsigned char> answerReverse (const std::vector<unsigned char>& answer);

			bool d_prefix;
	};

}

#endif /* LOGICALACCESS_ISO7816RPLETHREADERCARDADAPTER_HPP */

 

