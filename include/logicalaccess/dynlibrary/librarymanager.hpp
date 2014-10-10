#ifndef LIBRARYMANAGER_HPP__
#define LIBRARYMANAGER_HPP__

#include <string>
#include <list>

#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#if UNIX
#define LIBLOGICALACCESS_API_DLL
#else
#ifdef LIBLOGICALACCESS_EXPORTS_DLL
#define LIBLOGICALACCESS_API_DLL __declspec(dllexport)
#else
#define LIBLOGICALACCESS_API_DLL __declspec(dllimport)
#endif
#endif

namespace logicalaccess
{
    class LIBLOGICALACCESS_API_DLL LibraryManager
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
        static LibraryManager *getInstance()
        {
            if (NULL == _singleton)
                _singleton = new LibraryManager();
            return _singleton;
        }

        void* getFctFromName(std::string fctname, LibraryType libraryType);
        static  LibraryManager *_singleton;

        boost::shared_ptr<ReaderProvider> getReaderProvider(const std::string& readertype);
        boost::shared_ptr<Chip> getCard(const std::string& cardtype);
        boost::shared_ptr<Commands> getCommands(const std::string& extendedtype);
        static boost::shared_ptr<DataTransport> getDataTransport(const std::string& transporttype);
        boost::shared_ptr<KeyDiversification> getKeyDiversification(const std::string& keydivtype);

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