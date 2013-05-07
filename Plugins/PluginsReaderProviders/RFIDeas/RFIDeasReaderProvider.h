/**
 * \file RFIDeasReaderProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RFIDeas card reader provider. 
 */

#ifndef LOGICALACCESS_READERRFIDEAS_PROVIDER_H
#define LOGICALACCESS_READERFIDEAS_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "RFIDeasReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	#define READER_RFIDEAS		"RFIDeas"

	/**
	 * \brief RFIDeas Reader Provider class.
	 */
	class LIBLOGICALACCESS_API RFIDeasReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the RFIDeasReaderProvider instance. Singleton because we can only have one communication, and RFIDeas can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<RFIDeasReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~RFIDeasReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_RFIDEAS; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "RFIDeas"; };

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
			RFIDeasReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERRFIDEAS_PROVIDER_H */

