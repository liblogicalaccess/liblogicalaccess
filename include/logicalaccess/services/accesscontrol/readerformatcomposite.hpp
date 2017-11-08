/**
 * \file readerformatcomposite.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader format composite.
 */

#ifndef LOGICALACCESS_READERFORMATCOMPOSITE_HPP
#define LOGICALACCESS_READERFORMATCOMPOSITE_HPP

#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>

namespace logicalaccess
{
/**
 * \brief A reader format composite class. This is a card format composite associated to a
 * specific reader configuration.
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
    std::shared_ptr<Format> readFormat() const;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     */
    void serialize(boost::property_tree::ptree &parentNode) override;

    /**
     * \brief UnSerialize a XML node to the current object.
     * \param node The XML node.
     */
    void unSerialize(boost::property_tree::ptree &node) override;

    /**
     * \brief Get the default Xml Node name for this object.
     * \return The Xml node name.
     */
    std::string getDefaultXmlNodeName() const override;

    /**
     * \brief Get the reader configuration.
     * \return The reader configuration.
     */
    std::shared_ptr<ReaderConfiguration> getReaderConfiguration() const;

    /*
     * \brief Set the reader configuration.
     * \param provider The reader configuration.
     */
    void setReaderConfiguration(std::shared_ptr<ReaderConfiguration> readerConfig);

    /**
     * \brief Get the cards format composite.
     * \return The cards foramt composite.
     */
    std::shared_ptr<CardsFormatComposite> getCardsFormatComposite() const;

    /*
     * \brief Set the cards format composite.
     * \param provider The cards format composite.
     */
    void setCardsFormatComposite(std::shared_ptr<CardsFormatComposite> composite);

  protected:
    /**
     * \brief The cards foramt composite.
     */
    std::shared_ptr<CardsFormatComposite> d_cardsFormatComposite;

    /**
     * \brief The reader configuration.
     */
    std::shared_ptr<ReaderConfiguration> d_readerConfiguration;
};
}

#endif /* LOGICALACCESS_READERFORMATCOMPOSITE_HPP */