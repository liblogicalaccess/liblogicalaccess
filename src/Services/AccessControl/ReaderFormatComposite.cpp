/**
 * \file readerformatcomposite.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader format composite.
 */

#include "logicalaccess/services/accesscontrol/readerformatcomposite.hpp"
#include "logicalaccess/cards/accessinfo.hpp"


namespace logicalaccess
{
	ReaderFormatComposite::ReaderFormatComposite()
	{
		d_readerConfiguration.reset(new ReaderConfiguration());
	}

	ReaderFormatComposite::~ReaderFormatComposite()
	{

	}	

	boost::shared_ptr<Format> ReaderFormatComposite::readFormat()
	{
		boost::shared_ptr<Format> fcopy;
		if (d_readerConfiguration && d_cardsFormatComposite)
		{
			d_cardsFormatComposite->setReaderUnit(d_readerConfiguration->getReaderUnit());
			fcopy = d_cardsFormatComposite->readFormat();
		}
		return fcopy;
	}

	void ReaderFormatComposite::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		if (d_readerConfiguration && d_cardsFormatComposite)
		{
			d_readerConfiguration->serialize(node);
			d_cardsFormatComposite->serialize(node);
		}

		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void ReaderFormatComposite::unSerialize(boost::property_tree::ptree& node)
	{
		INFO_SIMPLE_("Unserializing reader format composite...");

		d_readerConfiguration.reset(new ReaderConfiguration());
		d_readerConfiguration->unSerialize(node.get_child(d_readerConfiguration->getDefaultXmlNodeName()));

		d_cardsFormatComposite.reset(new CardsFormatComposite());
		if (!d_readerConfiguration->getReaderUnit())
		{
			d_readerConfiguration->setReaderUnit(d_readerConfiguration->getReaderProvider()->createReaderUnit());
		}
		d_cardsFormatComposite->setReaderUnit(d_readerConfiguration->getReaderUnit());						

		d_cardsFormatComposite->unSerialize(node.get_child(d_cardsFormatComposite->getDefaultXmlNodeName()));
	}

	std::string ReaderFormatComposite::getDefaultXmlNodeName() const
	{
		return "ReaderFormatComposite";
	}

	boost::shared_ptr<ReaderConfiguration> ReaderFormatComposite::getReaderConfiguration() const
	{
		return d_readerConfiguration;
	}

	void ReaderFormatComposite::setReaderConfiguration(boost::shared_ptr<ReaderConfiguration> readerConfig)
	{
		d_readerConfiguration = readerConfig;
	}

	boost::shared_ptr<CardsFormatComposite> ReaderFormatComposite::getCardsFormatComposite() const
	{
		return d_cardsFormatComposite;
	}

	void ReaderFormatComposite::setCardsFormatComposite(boost::shared_ptr<CardsFormatComposite> composite)
	{
		d_cardsFormatComposite = composite;
	}
}

