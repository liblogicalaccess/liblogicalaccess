/**
 * \file AdmittoReaderUnitConfiguration.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Admitto Reader unit configuration.
 */

#ifndef LOGICALACCESS_ADMITTOREADERUNITCONFIGURATION_H
#define LOGICALACCESS_ADMITTOREADERUNITCONFIGURATION_H

#include "logicalaccess/ReaderProviders/ReaderUnitConfiguration.h"

namespace logicalaccess
{	
	/**
	 * \brief The Admitto reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API AdmittoReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			AdmittoReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~AdmittoReaderUnitConfiguration();

			/**
			 * \brief Reset the configuration to default values
			 */
			virtual void resetConfiguration();

			/**
			 * \brief Serialize the current object to XML.
			 * \param parentNode The parent node.
			 */
			virtual void serialize(boost::property_tree::ptree& parentNode);

			/**
			 * \brief UnSerialize a XML node to the current object.
			 * \param node The XML node.
			 */
			virtual void unSerialize(boost::property_tree::ptree& node);

			/**
			 * \brief Get the default Xml Node name for this object.
			 * \return The Xml node name.
			 */
			virtual std::string getDefaultXmlNodeName() const;

		protected:

	};
}

#endif
