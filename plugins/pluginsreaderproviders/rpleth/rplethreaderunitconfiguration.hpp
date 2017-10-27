/**
 * \file rplethreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth Reader unit configuration.
 */

#ifndef LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_RPLETHREADERUNITCONFIGURATION_HPP

#include "../iso7816/iso7816readerunitconfiguration.hpp"

namespace logicalaccess
{
/**
 * \brief Represent the rpleth firmware version.
 */
typedef enum { WIEGAND = 0x01, PROXY = 0x02 } RplethMode;

/**
 * \brief The Rpleth reader unit configuration base class.
 */
class LIBLOGICALACCESS_API RplethReaderUnitConfiguration
    : public ISO7816ReaderUnitConfiguration
{
  public:
    /**
     * \brief Constructor.
     */
    RplethReaderUnitConfiguration();

    /**
     * \brief Destructor.
     */
    virtual ~RplethReaderUnitConfiguration();

    /**
     * \brief Reset the configuration to default values
     */
    void resetConfiguration() override;

    /**
     * \brief Set the wiegand configuration to param values
     * \param offset The offset in wiegand communication
     * \param lenght The lenght of csn in wiegand communication
     */
    void setWiegandConfiguration(unsigned char offset, unsigned char lenght);

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
     * \brief Get the offset in wiegand communication.
     * \return The offset in wiegand communication.
     */
    unsigned char getOffset() const;

    /**
     * \brief Set the offset in wiegand communication.
     * \param offset The new offset in wiegand communication.
     */
    void setOffset(unsigned char offset);

    /**
     * \brief Get the length of csn in wiegand communication.
     * \return The length of csn in wiegand communication.
     */
    unsigned char getLength() const;

    /**
     * \brief Set the lenght of csn in wiegand communication.
     * \param length The new length of csn in wiegand communication.
     */
    void setLength(unsigned char length);

    /**
     * \brief Get the mode of the reader.
     * \return The mode of the reader.
     */
    RplethMode getMode() const;

    /**
     * \brief Set the type of the reader.
     * \param mode The new type of the reader.
     */
    void setMode(RplethMode mode);

  protected:
    /**
     * \brief Represent offset use to compute the CSN.
     */
    unsigned char d_offset;

    /**
     * \brief Represent length of CSN.
     */
    unsigned char d_length;

    /**
     * \brief Represent the Rpleth firmware version currently use.
     */
    RplethMode d_mode;
};
}

#endif