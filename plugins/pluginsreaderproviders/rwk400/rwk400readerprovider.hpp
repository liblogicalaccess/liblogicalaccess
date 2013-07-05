/**
 * \file rwk400readerprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 card reader provider. 
 */

#ifndef LOGICALACCESS_READERRWK400_PROVIDER_HPP
#define LOGICALACCESS_READERRWK400_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "rwk400readerunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"


namespace logicalaccess
{	
	#define READER_RWK400	"Rwk400"

	/**
	 * \brief Rpleth Reader Provider class.
	 */
	class LIBLOGICALACCESS_API Rwk400ReaderProvider : public ReaderProvider
	{
		public:

			/**
			 * \brief Get the A3MLGM5600ReaderProvider instance. Singleton because we can only have one TCP communication, and Rpleth can't have shared access, so we share the same reader unit.
			 */
			static boost::shared_ptr<Rwk400ReaderProvider> getSingletonInstance();

			/**
			 * \brief Get a new RplethReaderProvider instance.
			 */
			boost::shared_ptr<Rwk400ReaderProvider> createInstance();

			/**
			 * \brief Destructor.
			 */
			~Rwk400ReaderProvider();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			virtual std::string getRPType() const { return READER_RWK400; };

			/**
			 * \brief Get the reader provider name.
			 * \return The reader provider name.
			 */
			virtual std::string getRPName() const { return "Rwk400"; };

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
			Rwk400ReaderProvider();			

			/**
			 * \brief The reader list.
			 */
			ReaderList d_readers;			
	};

}

#endif /* LOGICALACCESS_READERRWK400_PROVIDER_HPP */

