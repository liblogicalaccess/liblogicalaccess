#ifndef LIBRARYMANAGER_HPP__
#define LIBRARYMANAGER_HPP__

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>

namespace logicalaccess
{

class AccessControlCardService;

class LLA_CORE_API LibraryManager
{
  public:
    enum LibraryType
    {
        READERS_TYPE = 0,
        CARDS_TYPE   = 1,
        UNIFIED_TYPE = 2
    };

  private:
    LibraryManager() = default;
    ~LibraryManager() = default;

    static bool hasEnding(const std::string &fullString, const std::string &ending);

    /**
     * \brief Finds a loaded library providing the requested symbol.
     *
     * The returned shared pointer keeps the library alive while the caller uses its symbol.
     *
     * Must be called while holding mutex_.
     *
     * \param fctname Symbol to find.
     * \param libraryType Expected plugin category.
     * \param symbol Receives the resolved symbol address.
     * \return The library providing the symbol, or an empty pointer.
     */
    std::shared_ptr<IDynLibrary> findLibraryForSymbol(const std::string &fctname, LibraryType libraryType, void **symbol);

  public:
    static LibraryManager *getInstance();

    /**
     * \brief Retrieves a function address from a loaded plugin.
     *
     * The returned pointer remains valid while its library remains loaded.
     * Replaced libs are retained until LibraryManager destruction because legacy API doesn't expose an ownership handle
     */
    void *getFctFromName(const std::string &fctname, LibraryType libraryType);

    std::shared_ptr<ReaderProvider> getReaderProvider(const std::string &readertype);

    /**
     * \brief Creates a ReaderUnit using an available reader plugin.
     *
     * Returns an empty pointer if no plugin can create the requested reader.
     */
    std::shared_ptr<ReaderUnit> getReader(const std::string &readerName);

    std::shared_ptr<Chip> getCard(const std::string &cardtype);

    std::shared_ptr<Commands> getCommands(const std::string &extendedtype);

    /**
     * \brief Retrieves a PKCS-backed AES crypto service.
     *
     * The PKCS library path is passed to the plugin implementation.
     *
     * Environment properties are temporarily configured during plugin initialization.
     * Existing environment variables are not overwritten.
     */
    std::shared_ptr<IAESCryptoService> getPKCSAESCrypto(const std::string &pkcs_library_shared_object_path,
                                                        const std::unordered_map<std::string, std::string> &pkcs_properties);

    static std::shared_ptr<DataTransport> getDataTransport(const std::string &transporttype);

    std::shared_ptr<KeyDiversification> getKeyDiversification(const std::string &keydivtype);

    /**
     * Retrieve a Service object for a given CardServiceType.
     *
     * This gives a chance to library to offer a customized service based
     * on the chip/reader combination.
     *
     * Use case: Fetch an UIDChanger service from the LLA-NFC plugin for a
     * Mifare Classic card.
     */
    std::shared_ptr<CardService> getCardService(std::shared_ptr<Chip> chip, CardServiceType type);

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

    /**
     * \brief Scans configured plugin folders and loads matching libraries.
     *
     * When two valid libraries have the same filename, last successfully loaded library replaces the previous active entry.
     */
    void scanPlugins();

  protected:
    std::vector<std::string> getAvailablePlugins(LibraryType libraryType);
    static void getAvailablePlugins(std::vector<std::string> &plugins, getobjectinfoat objectinfoptr);

  private:
    // Internal helpers called while mutex_ is held by scanPlugins()
    void scanPluginFolder(const std::string &folder);
    void processPluginFile(const boost::filesystem::path &file);
    bool isPluginCandidate(const boost::filesystem::path &file, const std::string &filename) const;
    void loadPlugin(const boost::filesystem::path &file, const std::string &filename);
    void registerPlugin(const std::string &filename, const std::shared_ptr<IDynLibrary> &library);

    mutable std::recursive_mutex mutex_;

    /**
     * \brief Active plugin libraries, indexed by filename.
     *
     * Shared ownership ensures that plugin-created objects can keep their originating library loaded after a replacement.
     */
    std::map<std::string, std::shared_ptr<IDynLibrary>> libLoaded_;

    /**
     * \brief Previously replaced libraries.
     *
     * getFctFromName() exposes raw function pointers without an ownership handle.
     * Retaining replaced libraries prevents those pointers from becoming invalid during the lifetime of LibraryManager.
     *
     * Plugin-created objects may independently extend library lifetimes.
     */
    std::vector<std::shared_ptr<IDynLibrary>> retiredLibraries_;

    static const std::string enumType[3];
};
}

#endif