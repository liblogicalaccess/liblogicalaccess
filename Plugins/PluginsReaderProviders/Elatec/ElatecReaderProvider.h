/**
 * \file ElatecReaderProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Elatec card reader provider. 
 */

#ifndef LOGICALACCESS_READERELATEC_PROVIDER_H
#define LOGICALACCESS_READERELATEC_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "ElatecReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief Elatec Reader Provider class.
	 */
	class LIBLOGICALACCESS_API ElatecReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the ElatecReaderProvider instance. Singleton because we can only have one COM communication, and Elatec can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<ElatecReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~ElatecReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return "Elatec"; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Elatec"; };

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
			ElatecReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERELATEC_PROVIDER_H */

