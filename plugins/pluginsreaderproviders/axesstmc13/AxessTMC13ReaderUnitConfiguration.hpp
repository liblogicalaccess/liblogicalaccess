/**
 * \file axesstmc13readerunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AxessTMC13 Reader unit configuration.
 */

#ifndef LOGICALACCESS_AXESSTMC13READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_AXESSTMC13READERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"


namespace logicalaccess
{	
	/**
	 * \brief The AxessTMC13 reader unit configuration base class.
	 */
	class LIBLOGICALACCESS_API AxessTMC13ReaderUnitConfiguration : public ReaderUnitConfiguration
	{
		public:

			/**
			 * \brief Constructor.
			 */
			AxessTMC13ReaderUnitConfiguration();

			/**
			 * \brief Destructor.
			 */
			virtual ~AxessTMC13ReaderUnitConfiguration();

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
