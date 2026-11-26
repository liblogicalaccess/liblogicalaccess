#include <logicalaccess/dynlibrary/librarymanager.hpp>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

#include <algorithm>
#include <cstring>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

// TODO: Data transport should also be through plug-in
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <logicalaccess/readerproviders/udpdatatransport.hpp>
#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/myexception.hpp>
#include <logicalaccess/utils.hpp>

namespace logicalaccess
{

namespace
{

/**
 * \brief Converts a dynamic library symbol into its function pointer type.
 *
 * The dynamic loader returns a void pointer.
 * Conversion to the appropriate function pointer is required before invoking an exported plugin function.
 */
template <typename FunctionPointer>
FunctionPointer toFunctionPointer(void *symbol)
{
    return reinterpret_cast<FunctionPointer>(symbol);
}

const char *const kLibraryNameEntry = "getLibraryName";
const char *const kChipInfoEntry    = "getChipInfoAt";
const char *const kReaderInfoEntry  = "getReaderInfoAt";

/**
 * \brief Resolves a plugin symbol to a typed function pointer.
 *
 * Missing symbols are logged and reported as nullptr.
 *
 * Unexpected dynamic loader exceptions are deliberately not swallowed.
 *
 * \param library Library in which to search.
 * \param symbolName Name of the exported symbol.
 * \return The resolved function pointer, or nullptr if unavailable.
 */
template <typename FunctionPointer>
FunctionPointer resolveFunction(const std::shared_ptr<IDynLibrary> &library, const std::string &symbolName)
{
    if (!library)
        return nullptr;

    if (!library->hasSymbol(symbolName.c_str()))
    {
        LOG(LogLevel::PLUGINS) << "Plugin '" << library->getName() << "' doesn't provide symbol '" << symbolName << "'";
        return nullptr;
    }

    void *symbol = library->getSymbol(symbolName.c_str());

    if (symbol == nullptr)
    {
        LOG(LogLevel::PLUGINS) << "Plugin '" << library->getName()  << "' returned a null address for symbol '"
                               << symbolName << "'";
        return nullptr;
    }

    return toFunctionPointer<FunctionPointer>(symbol);
}

/**
 * \brief Keeps the originating plugin loaded for the lifetime of an object.
 *
 * Plugin created objects can contain virtual tables, destructors and custom deleters inside their shared library
 *
 * The library must remain loaded until the object has been destroyed.
 *
 * Member declaration order is intentional and are destroyed in reverse declaration order.
 * Object must be destroyed before library.
 */
template <typename T>
std::shared_ptr<T> retainLibrary(const std::shared_ptr<T> &object, const std::shared_ptr<IDynLibrary> &library)
{
    if (!object || !library)
        return object;

    struct PluginObjectOwner
    {
        // Keep the library alive until after the plugin object is destroyed.
        std::shared_ptr<IDynLibrary> library;
        std::shared_ptr<T> object; // library and object order matters : object must be destroyed before library is unloaded

        PluginObjectOwner(const std::shared_ptr<T> &obj, const std::shared_ptr<IDynLibrary> &lib)
            : library(lib)
            , object(obj)
        {
        }
    };

    std::shared_ptr<PluginObjectOwner> owner = std::make_shared<PluginObjectOwner>(object, library);

    return std::shared_ptr<T>(owner, object.get());
}

/**
 * \brief Invokes a plugin function and preserves its exception semantics.
 *
 * Plugin execution exceptions must not be silently converted into empty results.
 * They are logged with their origin and rethrown unchanged.
 */
template <typename Function>
void invokePlugin(const std::string &libraryName, const std::string &symbolName, Function function)
{
    try
    {
        function();
    }
    catch (const std::exception &e)
    {
        LOG(LogLevel::PLUGINS) << "Plugin function '" << symbolName << "' failed in '" << libraryName << "': " << e.what();
        throw;
    }
    catch (...)
    {
        LOG(LogLevel::PLUGINS) << "Plugin function '" << symbolName << "' failed in '" << libraryName
                               << "': unknown exception";
        throw;
    }
}

} // namespace

// ----------------------------------------
// Static members
// ----------------------------------------

const std::string LibraryManager::enumType[3] = {"readers", "cards", "unified"};

// ----------------------------------------
// Singleton
// ----------------------------------------

LibraryManager *LibraryManager::getInstance()
{
    static LibraryManager instance;
    return &instance;
}

// ----------------------------------------
// Utility
// ----------------------------------------

bool LibraryManager::hasEnding(const std::string &fullString, const std::string &ending)
{
    return fullString.length() >= ending.length() &&
           fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
}

// ----------------------------------------
// Symbol lookup
// ----------------------------------------

std::shared_ptr<IDynLibrary>
LibraryManager::findLibraryForSymbol(const std::string &fctname, LibraryType libraryType, void **symbol)
{
    if (symbol == nullptr)
        return std::shared_ptr<IDynLibrary>();

    *symbol = nullptr;

    if (libraryType < READERS_TYPE || libraryType > UNIFIED_TYPE)
    {
        LOG(LogLevel::PLUGINS) << "Invalid library type : " << static_cast<int>(libraryType);
        return std::shared_ptr<IDynLibrary>();
    }

    if (libLoaded_.empty())
        scanPlugins();

    const std::string suffix = enumType[libraryType] + EXTENSION_LIB;

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library || !hasEnding(entry.first, suffix))
            continue;

        try
        {
            void *foundSymbol = library->getSymbol(fctname.c_str());

            if (foundSymbol != nullptr)
            {
                *symbol = foundSymbol;
                return library;
            }
        }
        catch (const std::exception &e)
        {
            LOG(LogLevel::PLUGINS) << "Symbol lookup '" << fctname << "' failed in '" << entry.first << "': " << e.what();
        }
        catch (...)
        {
            LOG(LogLevel::PLUGINS_ERROR) << "Unknown symbol lookup error for '" << fctname << "' in '" << entry.first << "'";
        }
    }

    return std::shared_ptr<IDynLibrary>();
}

void *LibraryManager::getFctFromName(const std::string &fctname, LibraryType libraryType)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    void *symbol = nullptr;

    findLibraryForSymbol(fctname, libraryType, &symbol);

    if (symbol == nullptr)
        LOG(LogLevel::PLUGINS) << "No plugin provides symbol '" << fctname << "'";

    return symbol;
}

// ----------------------------------------
// Reader provider
// ----------------------------------------

std::shared_ptr<ReaderProvider> LibraryManager::getReaderProvider(const std::string &readertype)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<ReaderProvider> result;
    const std::string symbolName = "get" + readertype + "Reader";

    void *symbol = nullptr;

    std::shared_ptr<IDynLibrary> library = findLibraryForSymbol(symbolName, READERS_TYPE, &symbol);

    if (!library || symbol == nullptr)
        return result;

    getprovider factory = toFunctionPointer<getprovider>(symbol);

    invokePlugin(library->getName(), symbolName, [&factory, &result]() { factory(&result); });

    if (!result)
        LOG(LogLevel::PLUGINS) << "Reader provider '" << readertype << "' returned no instance";

    return retainLibrary(result, library);
}

// ----------------------------------------
// Card
// ----------------------------------------

std::shared_ptr<Chip> LibraryManager::getCard(const std::string &cardtype)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<Chip> result;
    const std::string symbolName = "get" + cardtype + "Chip";

    void *symbol = nullptr;

    std::shared_ptr<IDynLibrary> library = findLibraryForSymbol(symbolName, CARDS_TYPE, &symbol);

    if (!library || symbol == nullptr)
        return result;

    getcard factory = toFunctionPointer<getcard>(symbol);

    invokePlugin(library->getName(), symbolName, [&factory, &result]() { factory(&result); });

    if (!result)
        LOG(LogLevel::PLUGINS) << "Chip '" << cardtype << "' returned no instance";

    return retainLibrary(result, library);
}

// ----------------------------------------
// Key diversification
// ----------------------------------------

std::shared_ptr<KeyDiversification> LibraryManager::getKeyDiversification(const std::string &keydivtype)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<KeyDiversification> result;
    const std::string symbolName = "get" + keydivtype + "Diversification";

    void *symbol = nullptr;

    std::shared_ptr<IDynLibrary> library = findLibraryForSymbol(symbolName, CARDS_TYPE, &symbol);

    if (!library || symbol == nullptr)
        return result;

    getdiversification factory = toFunctionPointer<getdiversification>(symbol);

    invokePlugin(library->getName(), symbolName, [&factory, &result]() { factory(&result); });

    if (!result)
        LOG(LogLevel::PLUGINS) << "Key diversification '" << keydivtype << "' returned no instance";

    return retainLibrary(result, library);
}

// ----------------------------------------
// Commands
// ----------------------------------------

std::shared_ptr<Commands> LibraryManager::getCommands(const std::string &extendedtype)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<Commands> result;
    const std::string symbolName = "get" + extendedtype + "Commands";

    LOG(LogLevel::INFOS) << "Trying to find commands : " << extendedtype;

    void *symbol = nullptr;

    std::shared_ptr<IDynLibrary> library = findLibraryForSymbol(symbolName, READERS_TYPE, &symbol);

    if (!library || symbol == nullptr)
        return result;

    LOG(LogLevel::INFOS) << "Found command " << extendedtype;

    getcommands factory = toFunctionPointer<getcommands>(symbol);

    invokePlugin(library->getName(), symbolName, [&factory, &result]() { factory(&result); });

    if (!result)
        LOG(LogLevel::PLUGINS) << "Commands '" << extendedtype << "' returned no instance";

    return retainLibrary(result, library);
}

// ----------------------------------------
// Data transport
// ----------------------------------------

std::shared_ptr<DataTransport> LibraryManager::getDataTransport(const std::string &transporttype)
{
    if (transporttype == TRANSPORT_SERIALPORT)
    {
        return std::make_shared<SerialPortDataTransport>();
    }

    if (transporttype == TRANSPORT_UDP)
    {
        return std::make_shared<UDPDataTransport>();
    }

    if (transporttype == TRANSPORT_TCP)
    {
        return std::make_shared<TCPDataTransport>();
    }

    LOG(LogLevel::PLUGINS) << "Unknown data transport requested : " << transporttype;

    return std::shared_ptr<DataTransport>();
}

// ----------------------------------------
// Available plugins
// ----------------------------------------

std::vector<std::string> LibraryManager::getAvailableCards()
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    return getAvailablePlugins(CARDS_TYPE);
}

std::vector<std::string> LibraryManager::getAvailableReaders()
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    return getAvailablePlugins(READERS_TYPE);
}

std::vector<std::string> LibraryManager::getAvailableDataTransports()
{
    return {TRANSPORT_SERIALPORT, TRANSPORT_UDP, TRANSPORT_TCP};
}

std::vector<std::string> LibraryManager::getAvailablePlugins(LibraryType libraryType)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<std::string> plugins;

    const char *symbolName = nullptr;

    if (libraryType == CARDS_TYPE)
    {
        symbolName = kChipInfoEntry;
    }
    else if (libraryType == READERS_TYPE)
    {
        symbolName = kReaderInfoEntry;
    }
    else
    {
        LOG(LogLevel::PLUGINS) << "getAvailablePlugins : unsupported library type " << static_cast<int>(libraryType);
        return plugins;
    }

    if (libLoaded_.empty())
        scanPlugins();

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library)
            continue;

        try
        {
            void *symbol = library->getSymbol(symbolName);

            if (symbol == nullptr)
                continue;

            auto objectInfo = toFunctionPointer<getobjectinfoat>(symbol);

            if (objectInfo != nullptr)
                getAvailablePlugins(plugins, objectInfo);
        }
        catch (const std::exception &e)
        {
            LOG(LogLevel::PLUGINS_ERROR) << "Failed to enumerate plugins in " << entry.first << ": " << e.what();
        }
        catch (...)
        {
            LOG(LogLevel::PLUGINS_ERROR)
                << "Unknown error enumerating plugins in " << entry.first;
        }
    }

    LOG(LogLevel::PLUGINS) << "Found " << plugins.size() << " available plugin(s) for type "
                           << static_cast<int>(libraryType);

    return plugins;
}

void LibraryManager::getAvailablePlugins(std::vector<std::string> &plugins, getobjectinfoat objectinfoptr)
{
    if (objectinfoptr == nullptr)
        return;

    // Guard against a misbehaving plugin that never returns false
    constexpr unsigned int kMaxPluginEntries = 4096;

    char objectname[PLUGINOBJECT_MAXLEN];
    void *objectPointer = nullptr;

    for (unsigned int index = 0; index < kMaxPluginEntries; ++index)
    {
        std::memset(objectname, 0, sizeof(objectname));
        objectPointer = nullptr;

        if (!objectinfoptr(index, objectname, sizeof(objectname), &objectPointer))
            return;

        const bool possiblyTruncated       = objectname[sizeof(objectname) - 1] != '\0';
        objectname[sizeof(objectname) - 1] = '\0';

        if (possiblyTruncated)
            LOG(LogLevel::PLUGINS) << "Plug-in name at index " << index << " may be truncated";

        if (objectname[0] == '\0')
        {
            LOG(LogLevel::PLUGINS) << "Plug-in at index " << index << " reported an empty name. Skipped.";
            continue;
        }

        std::string name(objectname);
        if (std::find(plugins.begin(), plugins.end(), name) != plugins.end())
        {
            LOG(LogLevel::PLUGINS) << "Duplicate plug-in name '" << name << "' ignored.";
            continue;
        }

        plugins.push_back(std::move(name));
    }

    LOG(LogLevel::PLUGINS_ERROR) << "Plug-in enumeration stopped after " << kMaxPluginEntries
                                 << " entries; the plug-in may be misbehaving.";
}

// ----------------------------------------
// Reader unit
// ----------------------------------------

std::shared_ptr<ReaderUnit> LibraryManager::getReader(const std::string &readerName)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // The idea here is simply to loop over all shared library
    // and opportunistically call the `getReaderUnit()` function if it exists, hoping
    // that some module will be able to fulfil our request.
    std::shared_ptr<ReaderUnit> result;

    if (libLoaded_.empty())
        scanPlugins();

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library)
            continue;

        using ReaderFactory = int (*)(const std::string &, std::shared_ptr<ReaderUnit> &);

        ReaderFactory factory = resolveFunction<ReaderFactory>(library, "getReaderUnit");

        if (!factory)
            continue;

        invokePlugin(entry.first, "getReaderUnit", [&factory, &readerName, &result]() { factory(readerName, result); });

        if (result)
        {
            LOG(LogLevel::INFOS) << "Reader '" << readerName << "' resolved by " << entry.first;
            return retainLibrary(result, library);
        }
    }

    LOG(LogLevel::WARNINGS) << "No plugin could create reader '" << readerName << "'";

    return result;
}

// ----------------------------------------
// Plugin scanning and loading
// ----------------------------------------

void LibraryManager::scanPlugins()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    const Settings *setting = Settings::getInstance();

    LOG(LogLevel::PLUGINS) << "Will scan " << setting->PluginFolders.size() << " folders.";

    for (const auto &folder : setting->PluginFolders)
        scanPluginFolder(folder);
}

void LibraryManager::scanPluginFolder(const std::string &folder)
{
    const boost::filesystem::path pluginDir(folder);

    boost::system::error_code ec;

    const bool pathExists = boost::filesystem::exists(pluginDir, ec) && !ec;
    const bool directoryExists = pathExists && boost::filesystem::is_directory(pluginDir, ec) && !ec;

    if (!directoryExists)
    {
        LOG(LogLevel::WARNINGS) << "Cannot find plug-in folder " << folder;
        return;
    }

    LOG(LogLevel::PLUGINS) << "Scanning library folder " << pluginDir.string() << " ...";

    try
    {
        const boost::filesystem::directory_iterator end;

        for (boost::filesystem::directory_iterator it(pluginDir); it != end; ++it)
            processPluginFile(it->path());
    }
    catch (const std::exception &e)
    {
        LOG(LogLevel::ERRORS) << "Failed to iterate plug-in folder " << pluginDir.string() << ": " << e.what();
    }
}

void LibraryManager::processPluginFile(const boost::filesystem::path &file)
{
    const std::string filename = file.filename().string();

    LOG(LogLevel::PLUGINS) << "Checking library " << filename << "...";

    try
    {
        if (!isPluginCandidate(file, filename))
            return;

        loadPlugin(file, filename);
    }
    catch (const std::exception &e)
    {
        LOG(LogLevel::ERRORS) << "Error while handling " << filename << ": " << e.what();
    }
}

bool LibraryManager::isPluginCandidate(const boost::filesystem::path &file, const std::string &filename) const
{
    boost::system::error_code statusError;

    const boost::filesystem::file_status status = boost::filesystem::symlink_status(file, statusError);

    if (statusError)
    {
        LOG(LogLevel::PLUGINS) << "Cannot inspect plugin file " << filename << ": " << statusError.message();
        return false;
    }

    const bool isFile = boost::filesystem::is_regular_file(status) || boost::filesystem::is_symlink(status);

    const std::string extension = EXTENSION_LIB;

    const bool matchesName = hasEnding(filename, enumType[CARDS_TYPE] + extension) ||
                             hasEnding(filename, enumType[READERS_TYPE] + extension) ||
                             hasEnding(filename, enumType[UNIFIED_TYPE] + extension);

    if (!isFile || file.extension() != extension || !matchesName)
    {
        LOG(LogLevel::PLUGINS) << "File " << filename << " does not match expected filenames. Skipped.";
        return false;
    }

    return true;
}

void LibraryManager::loadPlugin(const boost::filesystem::path &file,
                                const std::string &filename)
{
    /*
     * Load and validate before modifying the active registry.
     * Failing mustn't remove a previously loaded working library.
     */
    std::shared_ptr<IDynLibrary> library(newDynLibrary(file.string()));

    if (!library)
    {
        LOG(LogLevel::PLUGINS_ERROR) << "Failed to load library " << filename;
        return;
    }

    // A valid plugin must expose its libraryname entry point before it can be registered.
    if (library->getSymbol(kLibraryNameEntry) == nullptr)
    {
        LOG(LogLevel::PLUGINS) << "Cannot find library entry point in " << filename << ". Skipped.";
        return;
    }

    registerPlugin(filename, library);

    LOG(LogLevel::PLUGINS) << "Library " << filename << " loaded.";
}

void LibraryManager::registerPlugin(const std::string &filename, const std::shared_ptr<IDynLibrary> &library)
{
    /*
     * Preserve original filename based replacement behavior.
     * Last successfully loaded library with same filename becomes the active library.
     */
    auto existing = libLoaded_.find(filename);

    if (existing == libLoaded_.end())
    {
        libLoaded_.emplace(filename, library);
        return;
    }

    LOG(LogLevel::PLUGINS) << "Replacing previously loaded library " << filename;

    if (existing->second)
        retiredLibraries_.push_back(existing->second);

    existing->second = library;
}

// ----------------------------------------
// Card service
// ----------------------------------------

std::shared_ptr<CardService> LibraryManager::getCardService(std::shared_ptr<Chip> chip, CardServiceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<CardService> result;

    if (libLoaded_.empty())
        scanPlugins();

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library)
            continue;

        using CardServiceFactory = int (*)(std::shared_ptr<Chip>, std::shared_ptr<CardService> &, CardServiceType);

        CardServiceFactory factory = resolveFunction<CardServiceFactory>(library, "getCardService");

        if (!factory)
            continue;

        invokePlugin(entry.first, "getCardService", [&factory, &chip, &result, type]() { factory(chip, result, type); });

        if (result)
            return retainLibrary(result, library);
    }

    LOG(LogLevel::PLUGINS) << "No card service available for the requested chip";

    return result;
}

// ----------------------------------------
// Reader service
// ----------------------------------------

ReaderServicePtr LibraryManager::getReaderService(ReaderUnitPtr reader, ReaderServiceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ReaderServicePtr result;

    if (libLoaded_.empty())
        scanPlugins();

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library)
            continue;

        using ReaderServiceFactory = int (*)(ReaderUnitPtr, ReaderServicePtr &, ReaderServiceType);

        ReaderServiceFactory factory = resolveFunction<ReaderServiceFactory>(library, "getReaderService");

        if (!factory)
            continue;

        invokePlugin(entry.first, "getReaderService", [&factory, &reader, &result, type]()
                     { factory(reader, result, type); });

        if (result)
            return retainLibrary(result, library);
    }

    LOG(LogLevel::PLUGINS) << "No reader service available for the requested reader";

    return result;
}

// ----------------------------------------
// PKCS AES crypto service
// ----------------------------------------

std::shared_ptr<IAESCryptoService> LibraryManager::getPKCSAESCrypto(
    const std::string &pkcs_library_shared_object_path, const std::unordered_map<std::string, std::string> &pkcs_properties)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<IDynLibrary> cryptoLibrary;
    IAESCryptoServicePtr pkcs_crypto;

    if (libLoaded_.empty())
        scanPlugins();

    // Configure the environment before invoking the PKCS plugin. overwrite = 0 preserves existing environment variables
    for (const auto &prop : pkcs_properties)
    {
        if (portable_setenv(prop.first.c_str(), prop.second.c_str(), 0) != 0)
            LOG(LogLevel::PLUGINS) << "Failed to set environment variable " << prop.first;
    }

    for (const auto &entry : libLoaded_)
    {
        const std::shared_ptr<IDynLibrary> &library = entry.second;

        if (!library)
            continue;

        using PKCSAESCryptoFactory = void (*)(IAESCryptoServicePtr &, const std::string &);

        PKCSAESCryptoFactory factory = resolveFunction<PKCSAESCryptoFactory>(library, "getPKCSAESCrypto");

        if (!factory)
            continue;

        invokePlugin(entry.first, "getPKCSAESCrypto", [&factory, &pkcs_crypto, &pkcs_library_shared_object_path]()
                     { factory(pkcs_crypto, pkcs_library_shared_object_path); });

        if (pkcs_crypto)
        {
            cryptoLibrary = library;
            break;
        }
    }

    if (!pkcs_crypto)
    {
        LOG(LogLevel::ERRORS) << "Cannot retrieve PKCSAESCrypto implementation for " << pkcs_library_shared_object_path;
        throw LibLogicalAccessException("Cannot retrieve PKCSAESCrypto implementation");
    }

    LOG(LogLevel::INFOS) << "PKCS AES crypto service initialized from "
                         << pkcs_library_shared_object_path;
    return retainLibrary(pkcs_crypto, cryptoLibrary);
}

}