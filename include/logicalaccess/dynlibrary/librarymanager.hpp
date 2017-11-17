#ifndef LIBRARYMANAGER_HPP__
#define LIBRARYMANAGER_HPP__

#include <string>
#include <list>
#include <mutex>

#include <logicalaccess/dynlibrary/idynlibrary.hpp>

#if defined(UNIX) || defined(ANDROID)
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
class AccessControlCardService;

class LIBLOGICALACCESS_API_DLL LibraryManager
{
  public:
    enum LibraryType
    {
        READERS_TYPE = 0,
        CARDS_TYPE   = 1,
        UNIFIED_TYPE = 2
    };

  private:
    LibraryManager()
    {
    }
    ~LibraryManager()
    {
    }

    static bool hasEnding(std::string const &fullString, std::string ending);

  public:
    static LibraryManager *getInstance();

    void *getFctFromName(const std::string &fctname, LibraryType libraryType);

    std::shared_ptr<ReaderProvider> getReaderProvider(const std::string &readertype);

    /**
    * Attempt to find a suitable library that can build a ReaderUnit for
    * a reader named `readerName`.
    *
    * Returns the allocated ReaderUnit object or NULL on failure.
    */
    std::shared_ptr<ReaderUnit> getReader(const std::string &readerName);
    std::shared_ptr<Chip> getCard(const std::string &cardtype);
    std::shared_ptr<Commands> getCommands(const std::string &extendedtype);
    static std::shared_ptr<DataTransport>
    getDataTransport(const std::string &transporttype);
    std::shared_ptr<KeyDiversification>
    getKeyDiversification(const std::string &keydivtype);

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
    ReaderServicePtr getReaderService(ReaderUnitPtr reader, ReaderServiceType type);

    std::vector<std::string> getAvailableCards();

    std::vector<std::string> getAvailableReaders();

    static std::vector<std::string> getAvailableDataTransports();

    void scanPlugins();

  protected:
    std::vector<std::string> getAvailablePlugins(LibraryType libraryType);
    static void getAvailablePlugins(std::vector<std::string> &plugins,
                                    getobjectinfoat objectinfoptr);

  private:
    mutable std::recursive_mutex mutex_;
    std::map<std::string, IDynLibrary *> libLoaded;
    static const std::string enumType[3];
};
}

#endif
