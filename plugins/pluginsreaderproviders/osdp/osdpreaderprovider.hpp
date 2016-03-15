/**
 * \file osdpreaderprovider.hpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief OSDP card reader provider. 
 */

#ifndef LOGICALACCESS_OSDPREADER_PROVIDER_HPP
#define LOGICALACCESS_OSDPREADER_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "osdpreaderunit.hpp"

namespace logicalaccess
{	
	#define READER_OSDP		"OSDP"

	/**
	 * \brief OSDP Reader Provider class.
	 */
	class LIBLOGICALACCESS_API OSDPReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the STidSTReaderProvider instance. Singleton because we can only have one COM communication, and Deister can't have shared access, so we share the same reader unit.
			 */
			static std::shared_ptr<OSDPReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~OSDPReaderProvider();

			/**
			 * \brief Release the provider resources.
			 */
			virtual void release();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_OSDP; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "OSDP"; };

			/**
			 * \brief List all readers of the system.
			 * \return True if the list was updated, false otherwise.
			 */
			virtual bool refreshReaderList();

			/**
			 * \brief Get reader list for this reader provider.
			 * \return The reader list.
			 */
			virtual const ReaderList& getReaderList() { return d_readers; };					
			
			/**
			 * \brief Create a new reader unit for the reader provider.
			 * \return A reader unit.
			 */
			virtual std::shared_ptr<ReaderUnit> createReaderUnit();

		protected:

			/**
			 * \brief Constructor.
			 */
			OSDPReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_OSDPREADER_PROVIDER_HPP */

