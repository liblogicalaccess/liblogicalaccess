/**
 * \file readerconfiguration.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader configuration.
 */

#ifndef LOGICALACCESS_READERCONFIGURATION_HPP
#define LOGICALACCESS_READERCONFIGURATION_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <map>

using std::string;
using std::ostream;
using std::map;
using std::pair;

namespace logicalaccess
{
    /**
     * \brief A reader configuration class. Contains the reader provider and selected reader unit.
     */
    class LIBLOGICALACCESS_API ReaderConfiguration : public XmlSerializable
    {
    public:

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
         * \brief Get the reader provider.
         * \return The reader provider.
         */
        boost::shared_ptr<ReaderProvider> getReaderProvider() const;

        /**
         * \brief Get the reader unit.
         * \return The reader unit.
         */
        boost::shared_ptr<ReaderUnit> getReaderUnit() const;

        /*
         * \brief Set the reader provider.
         * \param provider The reader provider.
         */
        void setReaderProvider(boost::shared_ptr<ReaderProvider> provider);

        /*
         * \brief Set the reader unit.
         * \param provider The reader unit.
         */
        void setReaderUnit(boost::shared_ptr<ReaderUnit> unit);

    protected:

        /**
         * \brief The reader provider.
         */
        boost::shared_ptr<ReaderProvider> d_readerProvider;

        /**
         * \brief The reader unit.
         */
        boost::shared_ptr<ReaderUnit> d_ReaderUnit;
    };
}

#endif /* LOGICALACCESS_READERCONFIGURATION_HPP */