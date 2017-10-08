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
		void release() override;

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
		std::string getRPType() const override { return READER_OSDP; }

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
		std::string getRPName() const override { return "OSDP"; }

			/**
			 * \brief List all readers of the system.
			 * \return True if the list was updated, false otherwise.
			 */
		bool refreshReaderList() override;

			/**
			 * \brief Get reader list for this reader provider.
			 * \return The reader list.
			 */
		const ReaderList& getReaderList() override { return d_readers; }					
			
			/**
			 * \brief Create a new reader unit for the reader provider.
			 * \return A reader unit.
			 */
		std::shared_ptr<ReaderUnit> createReaderUnit() override;

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

