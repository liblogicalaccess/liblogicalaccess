/**
 * \file idondemandreaderunitconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief IdOnDemand Reader unit configuration.
 */

#ifndef LOGICALACCESS_IDONDEMANDREADERUNITCONFIGURATION_HPP
#define LOGICALACCESS_IDONDEMANDREADERUNITCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerunitconfiguration.hpp"

namespace logicalaccess
{
    /**
     * \brief The IdOnDemand reader unit configuration base class.
     */
    class LIBLOGICALACCESS_API IdOnDemandReaderUnitConfiguration : public ReaderUnitConfiguration
    {
    public:

        /**
         * \brief Constructor.
         */
        IdOnDemandReaderUnitConfiguration();

        /**
         * \brief Destructor.
         */
        virtual ~IdOnDemandReaderUnitConfiguration();

        /**
         * \brief Reset the configuration to default values
         */
	    void resetConfiguration() override;

        /**
         * \brief Serialize the current object to XML.
         * \param parentNode The parent node.
         */
	    void serialize(boost::property_tree::ptree& parentNode) override;

        /**
         * \brief UnSerialize a XML node to the current object.
         * \param node The XML node.
         */
	    void unSerialize(boost::property_tree::ptree& node) override;

        /**
         * \brief Get the default Xml Node name for this object.
         * \return The Xml node name.
         */
	    std::string getDefaultXmlNodeName() const override;

        /**
         * \brief Get the SDK authentication code.
         * \return The authentication code.
         */
        std::string getAuthCode() const;

        /**
         * \brief Set the SDK authentication code.
         * \param authCode The authentication code.
         */
        void setAuthCode(std::string authCode);

    protected:

        /**
         * \brief The authentication code
         */
        std::string d_authCode;
    };
}

#endif