/**
 * \file stidstrreaderprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief STidSTR card reader provider.
 */

#ifndef LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP
#define LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"

namespace logicalaccess
{
#define READER_STIDSTR		"STidSTR"

    /**
     * \brief STidSTR Reader Provider class.
     */
    class LIBLOGICALACCESS_API STidSTRReaderProvider : public ReaderProvider
    {
    public:

        /**
         * \brief Get the STidSTReaderProvider instance. Singleton because we can only have one COM communication, and Deister can't have shared access, so we share the same reader unit.
         */
        static std::shared_ptr<STidSTRReaderProvider> getSingletonInstance();

        /**
         * \brief Destructor.
         */
        ~STidSTRReaderProvider();

        /**
         * \brief Release the provider resources.
         */
        virtual void release();

        /**
         * \brief Get the reader provider type.
         * \return The reader provider type.
         */
        virtual std::string getRPType() const { return READER_STIDSTR; };

        /**
         * \brief Get the reader provider name.
         * \return The reader provider name.
         */
        virtual std::string getRPName() const { return "STid STR"; };

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
         * \return A reader unit.
         */
        virtual std::shared_ptr<ReaderUnit> createReaderUnit();

    protected:

        /**
         * \brief Constructor.
         */
        STidSTRReaderProvider();

        /**
         * \brief The reader list.
         */
        ReaderList d_readers;
    };
}

#endif /* LOGICALACCESS_READERSTIDSTR_PROVIDER_HPP */