/**
 * \file SCIELReaderProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief SCIEL card reader provider. 
 */

#ifndef LOGICALACCESS_READERSCIEL_PROVIDER_H
#define LOGICALACCESS_READERSCIEL_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "SCIELReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief SCIEL Reader Provider class.
	 */
	class LIBLOGICALACCESS_API SCIELReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the SCIELReaderProvider instance. Singleton because we can only have one COM communication, and the SCIEL can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<SCIELReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~SCIELReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return "SCIEL"; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "ELA SCIEL"; };

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
			SCIELReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERSCIEL_PROVIDER_H */

