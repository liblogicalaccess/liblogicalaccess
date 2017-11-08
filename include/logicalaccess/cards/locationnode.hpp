/**
 * \file locationnode.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location node.
 */

#ifndef LOGICALACCESS_LOCATIONNODE_HPP
#define LOGICALACCESS_LOCATIONNODE_HPP

#include <logicalaccess/cards/location.hpp>

#include <string>
#include <vector>
#include <iostream>

#include <memory>

namespace logicalaccess
{
/**
 * \brief A location node description. This describe a location on a hierachical view.
 */
class LIBLOGICALACCESS_API LocationNode
{
  public:
    /**
     * \brief Constructor.
     */
    LocationNode();

    /**
     * \brief Destructor.
     */
    virtual ~LocationNode();

    /**
     * \brief Set the node name.
     * \param name The node name.
     */
    void setName(const std::string &name);

    /**
     * \brief Get the node name.
     * \return The node name.
     */
    std::string getName() const;

    /**
     * \brief Get the childrens node.
     * \return The childrens node.
     */
    std::vector<std::shared_ptr<LocationNode>> &getChildrens();

    /**
     * \brief Set the parent node.
     * \param parent The parent node.
     */
    void setParent(std::weak_ptr<LocationNode> parent);

    /**
     * \brief Get the parent node.
     * \return The parent node.
     */
    std::shared_ptr<LocationNode> getParent() const;

    /**
     * \brief Set if authentication is needed.
     * \param needed Authentication needed.
     */
    void setNeedAuthentication(bool needed);

    /**
     * \brief Get if authentication is needed.
     * \return True if authentication is needed, false otherwise.
     */
    bool getNeedAuthentication() const;

    /**
     * \brief Set if the node has properties.
     * \param hasProperties The node has properties or not.
     */
    void setHasProperties(bool hasProperties);

    /**
     * \brief Get if the node has properties.
     * \return True if the node has properties, false otherwise.
     */
    bool getHasProperties() const;

    /**
     * \brief Set the node length.
     * \param length The node length.
     */
    void setLength(size_t length);

    /**
     * \brief Get the node length.
     * \return The node length.
     */
    size_t getLength() const;

    /**
     * \brief Set the associated location.
     * \param location The associated location.
     */
    void setLocation(std::shared_ptr<Location> location);

    /**
     * \brief Get the associated location.
     * \return The associated location.
     */
    std::shared_ptr<Location> getLocation() const;

    /**
     * \brief Set the byte unit.
     * \param unit The byte unit.
     */
    void setUnit(size_t unit);

    /**
     * \brief Get the byte unit.
     * \return The byte unit.
     */
    size_t getUnit() const;

    /**
     * \brief Set if read is possible.
     * \param can True if read is possible, false otherwise.
     */
    void setCanRead(bool can);

    /**
    * \brief Get if read is possible.
    * \return True if read is possible, false otherwise.
    */
    bool getCanRead() const;

    /**
    * \brief Set if write is possible.
    * \param can True if write is possible, false otherwise.
    */
    void setCanWrite(bool can);

    /**
    * \brief Get if write is possible.
    * \return True if write is possible, false otherwise.
    */
    bool getCanWrite() const;

  protected:
    /**
     * \brief The childrens node.
     */
    std::vector<std::shared_ptr<LocationNode>> d_childrens;

    /**
     * \brief The parent node.
     */
    std::weak_ptr<LocationNode> d_parent;

    /**
     * \brief Need authentication to access the node data.
     */
    bool d_needAuthentication;

    /**
     * \brief The node has properties or not.
     */
    bool d_hasProperties;

    /**
     * \brief The data node length.
     */
    size_t d_length;

    /**
     * \brief The current location node name.
     */
    std::string d_name;

    /**
     * \brief The associated location.
     */
    std::shared_ptr<Location> d_location;

    /**
     * \brief The byte unit.
     */
    size_t d_unit;

    /**
     * \brief Read capability.
     */
    bool d_can_read;

    /**
    * \brief Write capability.
    */
    bool d_can_write;
};
}

#endif /* LOGICALACCESS_LOCATIONNODE_HPP */