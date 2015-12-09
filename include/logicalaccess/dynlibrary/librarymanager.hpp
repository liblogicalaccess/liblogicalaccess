#ifndef LIBRARYMANAGER_HPP__
#define LIBRARYMANAGER_HPP__

#include <string>
#include <list>

#include "logicalaccess/dynlibrary/idynlibrary.hpp"

namespace logicalaccess
{
    class AccessControlCardService;

    class LIBLOGICALACCESS_API LibraryManager
    {
    public:
        enum LibraryType {
            READERS_TYPE = 0,
            CARDS_TYPE = 1,
            UNIFIED_TYPE = 2
        };
    private:
        LibraryManager() {};
        ~LibraryManager() {};

        static bool hasEnding(std::string const &fullString, std::string ending);

    public:
		static LibraryManager *getInstance();

        void* getFctFromName(const std::string &fctname, LibraryType libraryType);
        static  LibraryManager *_singleton;

        std::shared_ptr<ReaderProvider> getReaderProvider(const std::string& readertype);

        /**
         * Attempt to instantiate a reader unit corresponding to the `readerIdentifer`
         * parameter.
         *
         * This is done by looping through the plugins and calling the C function
         * `getReaderUnit()` exported by plugins.
         *
         * The `readerIdentifier` parameter either represents the readerName or its
         * USB information. If USB informations were extracted, the `readerIdentifier`
         * would be something like "usb_identity::1c34_8141" instead of a traditional
         * reader name.
         *
         * The `readerName` is the name of the reader as exposed by the underlying
         * PCSC library.
         *
         * This function either returns a readerUnit corresponding to the identifier,
         * or nullptr on failure.
         */
        std::shared_ptr<ReaderUnit> getReader(const std::string &readerIdentifier,
                                              const std::string &readerName) const;

        std::shared_ptr<Chip> getCard(const std::string& cardtype);
        std::shared_ptr<Commands> getCommands(const std::string& extendedtype);
        static std::shared_ptr<DataTransport> getDataTransport(const std::string& transporttype);
        std::shared_ptr<KeyDiversification> getKeyDiversification(const std::string& keydivtype);

        /**
         * Attempt to find a library that can instantiate an AccessControl service
         * object for the chip.
         *
         * Returns a new service, or nullptr on failure.
         *
         * Note: Should be removed in favor of getCardService with type = AccessControl.
         */
        std::shared_ptr<AccessControlCardService> getAccessControlCardService(
                std::shared_ptr<Chip> chip) const;

        /**
         * Retrieve a Service object for a given CardServiceType.
         *
         * This gives a chance to library to offer a customized service based
         * on the chip/reader combination.
         *
         * Use case: Fetch an UIDChanger service from the LLA-NFC plugin for a
         * Mifare Classic card.
         */
        std::shared_ptr<CardService> getCardService(std::shared_ptr<Chip> chip,
                                                    CardServiceType type);

        /**
         * Retrieve a Service object for a given ReaderServiceType.
         *
         * Similarly to getCardService(), this call gives LLA plugins a
         * chance to provide an implementation for a reader/service-type
         * combination.
         */
        ReaderServicePtr getReaderService(ReaderUnitPtr reader,
                                          ReaderServiceType type);

        std::vector<std::string> getAvailableCards();

        std::vector<std::string> getAvailableReaders();

        static std::vector<std::string> getAvailableDataTransports();

        void scanPlugins();

    protected:

        std::vector<std::string> getAvailablePlugins(LibraryType libraryType);
        static void getAvailablePlugins(std::vector<std::string>& plugins, getobjectinfoat objectinfoptr);

    private:
        std::map<std::string, IDynLibrary*> libLoaded;
        static const std::string enumType[3];
    };
}

#endif