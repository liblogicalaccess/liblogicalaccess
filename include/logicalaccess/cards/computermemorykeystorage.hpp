/**
 * \file computermemorykeystorage.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Computer memory key storage description.
 */

#ifndef LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP
#define LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP

#include <logicalaccess/cards/keystorage.hpp>

namespace logicalaccess
{
/**
 * \brief A computer memory key storage class.
 */
class LLA_CORE_API ComputerMemoryKeyStorage : public KeyStorage
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
    ComputerMemoryKeyStorage();

    virtual ~ComputerMemoryKeyStorage() = default;

    /**
     * \brief Get the key storage type.
     * \return The key storage type.
     */
    KeyStorageType getType() const override;

    /**
     * \brief Serialize the current object to XML.
     * \param parentNode The parent node.
     * \return The XML string.
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
};
}

#endif /* LOGICALACCESS_COMPUTERMEMORYKEYSTORAGE_HPP */