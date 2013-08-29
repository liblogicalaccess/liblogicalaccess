/**
 * \file ok5553readerprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief OK5553 card reader provider. 
 */

#ifndef LOGICALACCESS_READEROK5553_PROVIDER_HPP
#define LOGICALACCESS_READEROK5553_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "ok5553readerunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"


namespace logicalaccess
{	
	#define READER_OK5553		"OK5553"

	/**
	 * \brief OK5553 Reader Provider class.
	 */
	class LIBLOGICALACCESS_API OK5553ReaderProvider : public ReaderProvider
	{
		public:

			/**
			 * \brief Get the OK5553ReaderProvider instance.
			 */
			static boost::shared_ptr<OK5553ReaderProvider> getSingletonInstance();

			/**
			 * \brief Get a new OK5553ReaderProvider instance.
			 */
			boost::shared_ptr<OK5553ReaderProvider> createInstance();

			/**
			 * \brief Destructor.
			 */
			~OK5553ReaderProvider();

			/**
			 * \brief Release the provider resources.
			 */
			virtual void release();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_OK5553; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "OK5553"; };

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
			 * param ip The reader address
			 * param port The reader port
			 * \return A reader unit.
			 */
			virtual boost::shared_ptr<ReaderUnit> createReaderUnit();			

		protected:

			/**
			 * \brief Constructor.
			 */
			OK5553ReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READEROK5553_PROVIDER_HPP */

