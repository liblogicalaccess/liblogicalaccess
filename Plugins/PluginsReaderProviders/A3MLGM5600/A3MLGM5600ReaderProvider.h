/**
 * \file A3MLGM5600ReaderProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 card reader provider. 
 */

#ifndef LOGICALACCESS_READERA3MLGM5600_PROVIDER_H
#define LOGICALACCESS_READERA3MLGM5600_PROVIDER_H

#include "A3MLGM5600ReaderUnit.h"
#include "logicalaccess/ReaderProviders/ReaderProvider.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "logicalaccess/logs.h"


namespace logicalaccess
{	
	#define READER_A3MLGM5600		"A3MLGM5600"

	/**
	 * \brief A3MLGM5600 Reader Provider class.
	 */
	class LIBLOGICALACCESS_API A3MLGM5600ReaderProvider : public ReaderProvider
	{
		public:			

			/**
			 * \brief Get the A3MLGM5600ReaderProvider instance. Singleton because we can only have one UDP communication, and A3MLGM5600 can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<A3MLGM5600ReaderProvider> getSingletonInstance();

			/**
			 * \brief Destructor.
			 */
			~A3MLGM5600ReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_A3MLGM5600; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "A3M LGM5600"; };

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
			A3MLGM5600ReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERA3MLGM5600_PROVIDER_H */

