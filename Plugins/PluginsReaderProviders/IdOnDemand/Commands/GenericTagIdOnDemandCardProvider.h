/**
 * \file GenericTagIdOnDemandCardProvider.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic tag IdOnDemand card provider.
 */

#ifndef LOGICALACCESS_GENERICTAGIDONDEMANDCARDPROVIDER_H
#define LOGICALACCESS_GENERICTAGIDONDEMANDCARDPROVIDER_H

#include "GenericTagCardProvider.h"


namespace LOGICALACCESS
{
	/**
	 * \brief The Generic Tag IdOnDemand card provider class.
	 */
	class LIBLOGICALACCESS_API GenericTagIdOnDemandCardProvider : public GenericTagCardProvider
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			GenericTagIdOnDemandCardProvider();

			/**
			 * \brief Destructor.
			 */
			virtual ~GenericTagIdOnDemandCardProvider();

			/**
			 * \brief Get a card service for this card provider.
			 * \param serviceType The card service type.
			 * \return The card service.
			 */
			virtual boost::shared_ptr<CardService> getService(CardServiceType serviceType);

			/**
			 * \brief Get the reader/card adapter.
			 * \return The reader/card adapter.
			 */
			virtual boost::shared_ptr<ReaderCardAdapter> getReaderCardAdapter() const { return d_readerCardAdapter; };

			/**
			 * \brief Set the reader/card adapter.
			 * \param adapter The reader/card adapter.
			 */
			virtual void setReaderCardAdapter(boost::shared_ptr<ReaderCardAdapter> adapter) { d_readerCardAdapter = adapter; };

		protected:

			/**
			 * \brief The reader/card adapter.
			 */
			boost::shared_ptr<ReaderCardAdapter> d_readerCardAdapter;

	};
}

#endif
