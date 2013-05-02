/**
 * \file GunneboReaderProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Gunnebo card reader provider. 
 */

#ifndef LOGICALACCESS_READERGUNNEBO_PROVIDER_H
#define LOGICALACCESS_READERGUNNEBO_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "GunneboReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief Gunnebo Reader Provider class.
	 */
	class LIBLOGICALACCESS_API GunneboReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the GunneboReaderProvider instance. Singleton because we can only have one COM communication, and Gunnebo can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<GunneboReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~GunneboReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return "Gunnebo"; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Gunnebo CRS1306AA1"; };

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
			GunneboReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERGUNNEBO_PROVIDER_H */

