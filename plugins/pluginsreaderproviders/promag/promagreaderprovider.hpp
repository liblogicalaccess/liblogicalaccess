/**
 * \file promagreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Promag card reader provider. 
 */

#ifndef LOGICALACCESS_READERPROMAG_PROVIDER_HPP
#define LOGICALACCESS_READERPROMAG_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "promagreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"


namespace logicalaccess
{	
	#define READER_PROMAG		"Promag"

	/**
	 * \brief Promag Reader Provider class.
	 */
	class LIBLOGICALACCESS_API PromagReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the PromagReaderProvider instance. Singleton because we can only have one COM communication, and Promag can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<PromagReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~PromagReaderProvider();

			/**
			 * \brief Release the provider resources.
			 */
			virtual void release();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_PROMAG; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Promag"; };

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
			virtual boost::shared_ptr<ReaderUnit> createReaderUnit();			

		protected:

			/**
			 * \brief Constructor.
			 */
			PromagReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERPROMAG_PROVIDER_HPP */

