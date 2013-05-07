/**
 * \file ReaderFormatComposite.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader format composite.
 */

#ifndef LOGICALACCESS_READERFORMATCOMPOSITE_H
#define LOGICALACCESS_READERFORMATCOMPOSITE_H

#include "logicalaccess/ReaderProviders/ReaderConfiguration.h"
#include "logicalaccess/Services/AccessControl/CardsFormatComposite.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A reader format composite class. This is a card format composite associated to a specific reader configuration.
	 */
	class LIBLOGICALACCESS_API ReaderFormatComposite : public XmlSerializable
	{
		public:

			/**
			 * \brief Constructor.
			 */
			ReaderFormatComposite();

			/**
			 * \brief Destructor.
			 */
			virtual ~ReaderFormatComposite();			

			/**
			 * \brief Read format from a card.
			 * \return The format.
			 */
			boost::shared_ptr<Format> readFormat();

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

			/**
			 * \brief Get the reader configuration.
			 * \return The reader configuration.
			 */
			boost::shared_ptr<ReaderConfiguration> getReaderConfiguration() const;

			/*
			 * \brief Set the reader configuration.
			 * \param provider The reader configuration.
			 */
			void setReaderConfiguration(boost::shared_ptr<ReaderConfiguration> readerConfig);

			/**
			 * \brief Get the cards format composite.
			 * \return The cards foramt composite.
			 */
			boost::shared_ptr<CardsFormatComposite> getCardsFormatComposite() const;

			/*
			 * \brief Set the cards format composite.
			 * \param provider The cards format composite.
			 */
			void setCardsFormatComposite(boost::shared_ptr<CardsFormatComposite> composite);

	protected:

			/**
			 * \brief The cards foramt composite.
			 */
			boost::shared_ptr<CardsFormatComposite> d_cardsFormatComposite;

			/**
			 * \brief The reader configuration.
			 */
			boost::shared_ptr<ReaderConfiguration> d_readerConfiguration;
	};	
}

#endif /* LOGICALACCESS_READERFORMATCOMPOSITE_H */
