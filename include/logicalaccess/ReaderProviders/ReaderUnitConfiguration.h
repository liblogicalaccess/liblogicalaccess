/**
 * \file ReaderUnitConfiguration.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Reader unit configuration.
 */

#ifndef LOGICALACCESS_ReaderUnitCONFIGURATION_H
#define LOGICALACCESS_ReaderUnitCONFIGURATION_H

#include "logicalaccess/XmlSerializable.h"

namespace LOGICALACCESS
{
	/**
	 * \brief The reader unit configuration base class. Describe the configuration for a reader unit.
	 */
	class LIBLOGICALACCESS_API ReaderUnitConfiguration : public XmlSerializable
	{
		public:
			
			/**
			 * \brief Constructor.
			 */
			ReaderUnitConfiguration(std::string rpt);

			/**
			 * \brief Destructor.
			 */
			virtual ~ReaderUnitConfiguration();

			/**
			 * \brief Get the reader provider type.
			 * \return The reader provider type.
			 */
			std::string getRPType() const;

			/**
			 * \brief Reset the configuration to the default one.
			 */
			virtual void resetConfiguration() = 0;

		protected:

			/**
			 * \brief The associated reader provider type for the reader unit configuration.
			 */
			std::string d_readerProviderType;
	};
}

#endif
