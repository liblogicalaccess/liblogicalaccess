/**
 * \file rplethreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth card reader provider.
 */

#ifndef LOGICALACCESS_READERRPLETH_PROVIDER_HPP
#define LOGICALACCESS_READERRPLETH_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "rplethreaderunit.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
#define READER_RPLETH		"Rpleth"

    /**
     * \brief Rpleth Reader Provider class.
     */
    class LIBLOGICALACCESS_API RplethReaderProvider : public ReaderProvider
    {
    public:

        /**
         * \brief Get the RplethReaderProvider instance. Singleton because we can only have one TCP communication, and Rpleth can't have shared access, so we share the same reader unit.
         */
        static std::shared_ptr<RplethReaderProvider> getSingletonInstance();

        /**
         * \brief Get a new RplethReaderProvider instance.
         */
        std::shared_ptr<RplethReaderProvider> createInstance();

        /**
         * \brief Destructor.
         */
        ~RplethReaderProvider();

        /**
         * \brief Release the provider resources.
         */
        virtual void release();

        /**
         * \brief Get the reader provider type.
         * \return The reader provider type.
         */
        virtual std::string getRPType() const { return READER_RPLETH; };

        /**
         * \brief Get the reader provider name.
         * \return The reader provider name.
         */
        virtual std::string getRPName() const { return "Rpleth"; };

        /**
         * \brief List all readers of the system.
         * \return True if the list was updated, false otherwise.
         */
        virtual bool refreshReaderList();

        /**
         * \brief Get reader list for this reader provider.
         * \return The reader list.
         */
        virtual const ReaderList& getReaderList() { return d_readers; };

        /**
         * \brief Create a new reader unit for the reader provider.
         * param ip The reader address
         * param port The reader port
         * \return A reader unit.
         */
        virtual std::shared_ptr<ReaderUnit> createReaderUnit();

    protected:

        /**
         * \brief Constructor.
         */
        RplethReaderProvider();

        /**
         * \brief The reader list.
         */
        ReaderList d_readers;
    };
}

#endif /* LOGICALACCESS_READERRPLETH_PROVIDER_HPP */