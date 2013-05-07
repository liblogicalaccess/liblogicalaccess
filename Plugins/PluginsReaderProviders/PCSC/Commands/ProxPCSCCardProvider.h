/**
 * \file ProxCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#ifndef LOGICALACCESS_PROXPCSCCARDPROVIDER_H
#define LOGICALACCESS_PROXPCSCCARDPROVIDER_H

#include "ProxCardProvider.h"
#include "../ReaderCardAdapters/PCSCReaderCardAdapter.h"

#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;

namespace LOGICALACCESS
{
	class PCSCReaderProvider;

	/**
	 * \brief The HID Prox PC/SC card provider class.
	 */
	class LIBLOGICALACCESS_API ProxPCSCCardProvider : public ProxCardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			ProxPCSCCardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~ProxPCSCCardProvider();							

			/**
			 * \brief Read data on a specific Mifare location, using given Mifare keys.
			 * \param location The data location.			 
			 * \param data Will contain data after reading.
			 * \param dataLength Data's length.
			 * \param dataLengthBits Data's length to read in bits.
			 * \return Data length read in bits.
			 */
			virtual unsigned int readData(boost::shared_ptr<ProxLocation> location, void* data, size_t dataLength, unsigned int dataLengthBits);

			void setReaderCardAdapter(boost::shared_ptr<PCSCReaderCardAdapter> cardAdapter) { d_cardAdapter = cardAdapter; };

		protected:

			boost::shared_ptr<PCSCReaderCardAdapter> d_cardAdapter;
	};
}

#endif
