/**
 * \file rwk400readerunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 Reader unit configuration.
 */

#ifndef LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
	/**
	 * \brief The Rwk400 reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API Rwk400ReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			Rwk400ReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~Rwk400ReaderUnitConfiguration();

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
	};
}

#endif /* LOGICALACCESS_RWK400READERUNITCONFIGURATION_HPP */
