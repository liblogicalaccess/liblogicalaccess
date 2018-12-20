/**
 * \file readerconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader configuration.
 */

#ifndef LOGICALACCESS_READERCONFIGURATION_HPP
#define LOGICALACCESS_READERCONFIGURATION_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/xmlserializable.hpp>

namespace logicalaccess
{
/**
 * \brief A reader configuration class. Contains the reader provider and selected reader
 * unit.
 */
class LLA_CORE_API ReaderConfiguration : public XmlSerializable
{
  public:
#ifndef SWIG
    using XmlSerializable::serialize;
    using XmlSerializable::unSerialize;
#else
    /**
     * \brief Serialize object to Xml Node string.
     * \return The serialized object.
     */
    virtual std::string serialize();

    /**
     * \brief UnSerialize object from a Xml Node string.
     * \param xmlstring The Xml Node string.
     * \param rootNode The root node.
     * \return True on success, false otherwise.
     */
    virtual void unSerialize(const std::string &xmlstring, const std::string &rootNode);

    /**
     * \brief UnSerialize object from a Xml node.
     * \param node The Xml node.
     * \param rootNode The root node.
     * \return True on success, false otherwise.
     */
    virtual void unSerialize(boost::property_tree::ptree &node,
                             const std::string &rootNode);
#endif

    /**
     * \brief Constructor.
     */
    ReaderConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~ReaderConfiguration();

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
     * \brief Get the reader provider.
     * \return The reader provider.
     */
    std::shared_ptr<ReaderProvider> getReaderProvider() const;

    /**
     * \brief Get the reader unit.
     * \return The reader unit.
     */
    std::shared_ptr<ReaderUnit> getReaderUnit() const;

    /*
     * \brief Set the reader provider.
     * \param provider The reader provider.
     */
    void setReaderProvider(std::shared_ptr<ReaderProvider> provider);

    /*
     * \brief Set the reader unit.
     * \param provider The reader unit.
     */
    void setReaderUnit(std::shared_ptr<ReaderUnit> unit);

  protected:
    /**
     * \brief The reader provider.
     */
    std::shared_ptr<ReaderProvider> d_readerProvider;

    /**
     * \brief The reader unit.
     */
    std::shared_ptr<ReaderUnit> d_ReaderUnit;
};
}

#endif /* LOGICALACCESS_READERCONFIGURATION_HPP */