/**
 * \file a3mlgm5600readerunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 Reader unit configuration.
 */

#ifndef LOGICALACCESS_A3MLGM5600READERUNITCONFIGURATION_HPP
#define LOGICALACCESS_A3MLGM5600READERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
    /**
     * \brief The A3MLGM5600 reader unit configuration base class.
     */
    class LIBLOGICALACCESS_API A3MLGM5600ReaderUnitConfiguration : public ReaderUnitConfiguration
    {
    public:

        /**
         * \brief Constructor.
         */
        A3MLGM5600ReaderUnitConfiguration();

        /**
         * \brief Destructor.
         */
        virtual ~A3MLGM5600ReaderUnitConfiguration();

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

        /**
         * \brief Get the local port.
         * \return The local port.
         */
        int getLocalPort() const;

        /**
         * \brief Set the local port.
         * \param port The local port.
         */
        void setLocalPort(int port);

    protected:

        /**
         * \brief The location listening port.
         */
        int d_localPort;
    };
}

#endif