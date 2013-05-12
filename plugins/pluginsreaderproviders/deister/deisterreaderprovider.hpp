/**
 * \file deisterreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Deister card reader provider. 
 */

#ifndef LOGICALACCESS_READERDEISTER_PROVIDER_HPP
#define LOGICALACCESS_READERDEISTER_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "deisterreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"


namespace logicalaccess
{	
	#define READER_DEISTER		"Deister"

	/**
	 * \brief Deister Reader Provider class.
	 */
	class LIBLOGICALACCESS_API DeisterReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the DeisterReaderProvider instance. Singleton because we can only have one COM communication, and Deister can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<DeisterReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~DeisterReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_DEISTER; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Deister"; };

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
			DeisterReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERDEISTER_PROVIDER_HPP */

