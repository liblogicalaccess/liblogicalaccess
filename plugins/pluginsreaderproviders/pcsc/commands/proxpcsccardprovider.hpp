/**
 * \file proxcardprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Prox card profiles.
 */

#ifndef LOGICALACCESS_PROXPCSCCARDPROVIDER_HPP
#define LOGICALACCESS_PROXPCSCCARDPROVIDER_HPP

#include "proxcardprovider.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	class PCSCDataTransport;

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

			void setDataTransport(boost::shared_ptr<PCSCDataTransport> dataTransport) { d_dataTransport = dataTransport; };

		protected:

			boost::shared_ptr<PCSCDataTransport> d_dataTransport;
	};
}

#endif
