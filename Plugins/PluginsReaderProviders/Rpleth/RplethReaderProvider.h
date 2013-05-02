/**
 * \file RplethReaderProvider.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Rpleth card reader provider. 
 */

#ifndef LOGICALACCESS_READERRPLETH_PROVIDER_H
#define LOGICALACCESS_READERRPLETH_PROVIDER_H

#include "logicalaccess/ReaderProviders/ReaderProvider.h"
#include "RplethReaderUnit.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace LOGICALACCESS
{	
	/**
	 * \brief Rpleth Reader Provider class.
	 */
	class LIBLOGICALACCESS_API RplethReaderProvider : public ReaderProvider
	{
		public:

			/**
			 * \brief Get the A3MLGM5600ReaderProvider instance. Singleton because we can only have one TCP communication, and Rpleth can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<RplethReaderProvider> getSingletonInstance();

			/**
			 * \brief Get a new RplethReaderProvider instance.
			 */
			boost::shared_ptr<RplethReaderProvider> createInstance();

			/**
			 * \brief Destructor.
			 */
			~RplethReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return "Rpleth"; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Rpleth"; };

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
			RplethReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERELATEC_PROVIDER_H */

