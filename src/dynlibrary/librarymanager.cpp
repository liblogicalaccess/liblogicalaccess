#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <boost/filesystem.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

// TODO: Data transport should also be through plug-in
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/readerproviders/tcpdatatransport.hpp>
#include <logicalaccess/readerproviders/udpdatatransport.hpp>
#include <logicalaccess/cards/keydiversification.hpp>
#include <logicalaccess/plugins/llacommon/settings.hpp>
#include <logicalaccess/utils.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
const std::string LibraryManager::enumType[3] = {"readers", "cards", "unified"};

bool LibraryManager::hasEnding(std::string const &fullString, std::string ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 ==
                fullString.compare(fullString.length() - ending.length(), ending.length(),
                                   ending));
    }
    return false;
}

LibraryManager *LibraryManager::getInstance()
{
    static LibraryManager instance;
    return &instance;
}

void *LibraryManager::getFctFromName(const std::string &fctname, LibraryType libraryType)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    void *fct;
    boost::filesystem::directory_iterator end_iter;
    std::string extension = EXTENSION_LIB;

    if (libLoaded.empty())
        scanPlugins();

    for (std::map<std::string, IDynLibrary *>::iterator it = libLoaded.begin();
         it != libLoaded.end(); ++it)
    {
        try
        {
            if ((*it).second != nullptr &&
                hasEnding((*it).first, enumType[libraryType] + extension) &&
                (fct = (*it).second->getSymbol(fctname.c_str())) != nullptr)
                return fct;
        }
        catch (...)
        {
        }
    }

    return nullptr;
}

std::shared_ptr<ReaderProvider>
LibraryManager::getReaderProvider(const std::string &readertype)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::shared_ptr<ReaderProvider> ret;
    std::string fctname = "get" + readertype + "Reader";

    getprovider getpcscsfct;
    *(void **)(&getpcscsfct) = getFctFromName(fctname, READERS_TYPE);

    if (getpcscsfct)
    {
        getpcscsfct(&ret);
    }

    return ret;
}

std::shared_ptr<Chip> LibraryManager::getCard(const std::string &cardtype)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::shared_ptr<Chip> ret;
    std::string fctname = "get" + cardtype + "Chip";

    getcard getcardfct;
    *(void **)(&getcardfct) = getFctFromName(fctname, CARDS_TYPE);

    if (getcardfct)
    {
        getcardfct(&ret);
    }

    return ret;
}

std::shared_ptr<KeyDiversification>
LibraryManager::getKeyDiversification(const std::string &keydivtype)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::shared_ptr<KeyDiversification> ret;
    std::string fctname = "get" + keydivtype + "Diversification";

    getdiversification getdiversificationfct;
    *(void **)(&getdiversificationfct) = getFctFromName(fctname, CARDS_TYPE);

    if (getdiversificationfct)
    {
        getdiversificationfct(&ret);
    }

    return ret;
}

std::shared_ptr<Commands> LibraryManager::getCommands(const std::string &extendedtype)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::shared_ptr<Commands> ret;
    std::string fctname = "get" + extendedtype + "Commands";

    LOG(LogLevel::INFOS) << "Trying to find commands: " << extendedtype;
    getcommands getcommandsfct;
    *(void **)(&getcommandsfct) = getFctFromName(fctname, READERS_TYPE);
    if (getcommandsfct)
    {
        LOG(LogLevel::INFOS) << "Found command " << extendedtype;
        getcommandsfct(&ret);
    }

    return ret;
}

std::shared_ptr<DataTransport>
LibraryManager::getDataTransport(const std::string &transporttype)
{
    std::shared_ptr<DataTransport> ret;

    if (transporttype == TRANSPORT_SERIALPORT)
    {
        ret.reset(new SerialPortDataTransport());
    }
    else if (transporttype == TRANSPORT_UDP)
    {
        ret.reset(new UDPDataTransport());
    }
    else if (transporttype == TRANSPORT_TCP)
    {
        ret.reset(new TCPDataTransport());
    }

    return ret;
}

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
    std::vector<std::string> list;

    list.push_back(TRANSPORT_SERIALPORT);
    list.push_back(TRANSPORT_UDP);
    list.push_back(TRANSPORT_TCP);

    return list;
}

std::vector<std::string> LibraryManager::getAvailablePlugins(LibraryType libraryType)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::vector<std::string> plugins;
    void *fct;
    boost::filesystem::directory_iterator end_iter;
    std::string fctname;

    if (libraryType == CARDS_TYPE)
    {
        fctname = "getChipInfoAt";
    }
    else if (libraryType == READERS_TYPE)
    {
        fctname = "getReaderInfoAt";
    }

    if (libLoaded.empty())
        scanPlugins();

    for (std::map<std::string, IDynLibrary *>::iterator it = libLoaded.begin();
         it != libLoaded.end(); ++it)
    {
        try
        {
            if ((*it).second != nullptr &&
                (fct = (*it).second->getSymbol(fctname.c_str())) != nullptr)
            {
                getobjectinfoat objectinfoptr;
                *(void **)(&objectinfoptr) = fct;
                getAvailablePlugins(plugins, objectinfoptr);
            }
        }
        catch (...)
        {
        }
    }

    return plugins;
}

void LibraryManager::getAvailablePlugins(std::vector<std::string> &plugins,
                                         getobjectinfoat objectinfoptr)
{
    char objectname[PLUGINOBJECT_MAXLEN];
    memset(objectname, 0x00, sizeof(objectname));
    void *getobjectptr;

    unsigned int i = 0;
    while (objectinfoptr(i, objectname, sizeof(objectname), &getobjectptr))
    {
        plugins.push_back(std::string(objectname));
        memset(objectname, 0x00, sizeof(objectname));
        ++i;
    }
}

std::shared_ptr<ReaderUnit> LibraryManager::getReader(const std::string &readerName)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    // The idea here is simply to loop over all shared library
    // and opportunistically call the `getReaderUnit()` function if it exists, hoping
    // that some module will be able to fulfil our request.
    std::shared_ptr<ReaderUnit> readerUnit;

    if (libLoaded.empty())
        scanPlugins();

    for (auto &&itr : libLoaded)
    {
        IDynLibrary *lib = itr.second;

        if (lib->hasSymbol("getReaderUnit"))
        {
            int (*fptr)(const std::string &, std::shared_ptr<ReaderUnit> &) = nullptr;
            fptr = reinterpret_cast<decltype(fptr)>(lib->getSymbol("getReaderUnit"));
            assert(fptr);
            fptr(readerName, readerUnit);
            if (readerUnit)
                break;
        }
    }
    return readerUnit;
}

void LibraryManager::scanPlugins()
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    boost::filesystem::directory_iterator end_iter;
    std::string extension = EXTENSION_LIB;
    Settings *setting     = Settings::getInstance();
    std::string fctname   = "getLibraryName";

    LOG(LogLevel::PLUGINS) << "Will scan " << setting->PluginFolders.size()
                           << " folders.";
    for (std::vector<std::string>::iterator it = setting->PluginFolders.begin();
         it != setting->PluginFolders.end(); ++it)
    {
        boost::filesystem::path pluginDir(*it);
        if (exists(pluginDir) && is_directory(pluginDir))
        {
            LOG(LogLevel::PLUGINS) << "Scanning library folder " << pluginDir.string()
                                   << " ...";
            for (boost::filesystem::directory_iterator dir_iter(pluginDir);
                 dir_iter != end_iter; ++dir_iter)
            {
                LOG(LogLevel::PLUGINS) << "Checking library "
                                       << dir_iter->path().filename().string() << "...";
                try
                {
                    if ((boost::filesystem::is_regular_file(dir_iter->status()) ||
                         boost::filesystem::is_symlink(dir_iter->status())) &&
                        dir_iter->path().extension() == extension &&
                        (hasEnding(dir_iter->path().filename().string(),
                                   enumType[LibraryManager::CARDS_TYPE] + extension) ||
                         hasEnding(dir_iter->path().filename().string(),
                                   enumType[LibraryManager::READERS_TYPE] + extension) ||
                         hasEnding(dir_iter->path().filename().string(),
                                   enumType[LibraryManager::UNIFIED_TYPE] + extension)))
                    {
                        IDynLibrary *lib = newDynLibrary(dir_iter->path().string());
                        void *fct        = lib->getSymbol(fctname.c_str());
                        if (fct != nullptr)
                        {
                            LOG(LogLevel::PLUGINS) << "Library "
                                                   << dir_iter->path().filename().string()
                                                   << " loaded.";
                            libLoaded[dir_iter->path().filename().string()] = lib;
                        }
                        else
                        {
                            LOG(LogLevel::PLUGINS)
                                << "Cannot found library entry point in "
                                << dir_iter->path().filename().string() << ". Skipped.";
                            delete lib;
                        }
                    }
                    else
                    {
                        LOG(LogLevel::PLUGINS)
                            << "File " << dir_iter->path().filename().string()
                            << " does not match excepted filenames. Skipped.";
                    }
                }
                catch (const std::exception &e)
                {
                    LOG(LogLevel::ERRORS) << "Something bad happened when handling "
                                          << dir_iter->path().filename().string() << ": "
                                          << e.what();
                }
            }
        }
        else
        {
            LOG(LogLevel::WARNINGS) << "Cannot found plug-in folder " << (*it);
        }
    }
}

std::shared_ptr<CardService> LibraryManager::getCardService(std::shared_ptr<Chip> chip,
                                                            CardServiceType type)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::shared_ptr<CardService> srv;

    if (libLoaded.empty())
        scanPlugins();

    for (auto &&itr : libLoaded)
    {
        IDynLibrary *lib = itr.second;

        if (lib->hasSymbol("getCardService"))
        {
            int (*fptr)(std::shared_ptr<Chip>, std::shared_ptr<CardService> &,
                        CardServiceType) = nullptr;
            fptr = reinterpret_cast<decltype(fptr)>(lib->getSymbol("getCardService"));
            assert(fptr);
            fptr(chip, srv, type);
            if (srv)
                break;
        }
    }
    return srv;
}

ReaderServicePtr LibraryManager::getReaderService(ReaderUnitPtr reader,
                                                  ReaderServiceType type)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    ReaderServicePtr srv;

    if (libLoaded.empty())
        scanPlugins();

    for (auto &&itr : libLoaded)
    {
        IDynLibrary *lib = itr.second;

        if (lib->hasSymbol("getReaderService"))
        {
            int (*fptr)(ReaderUnitPtr, ReaderServicePtr &, ReaderServiceType) = nullptr;
            fptr = reinterpret_cast<decltype(fptr)>(lib->getSymbol("getReaderService"));
            assert(fptr);
            fptr(reader, srv, type);
            if (srv)
                break;
        }
    }
    return srv;
}

std::shared_ptr<IAESCryptoService>
LibraryManager::getPKCSAESCrypto(const std::string &env_PROTECCIO_CONF_DIR,
                                 const std::string &pkcs_library_shared_object_path)
{
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    IAESCryptoServicePtr pkcs_crypto;

    if (libLoaded.empty())
        scanPlugins();

    for (auto &&itr : libLoaded)
    {
        IDynLibrary *lib = itr.second;

        if (lib->hasSymbol("getPKCSAESCrypto"))
        {
            int (*fptr)(IAESCryptoServicePtr &, const std::string &pkcs_lib_path) =
                nullptr;
            fptr = reinterpret_cast<decltype(fptr)>(lib->getSymbol("getPKCSAESCrypto"));
            assert(fptr);

            // Configure environment for Atos NetHSM.
            portable_setenv("PROTECCIO_CONF_DIR", env_PROTECCIO_CONF_DIR.c_str(), 0);

            fptr(pkcs_crypto, pkcs_library_shared_object_path);
            if (pkcs_crypto)
                break;
        }
    }
    if (!pkcs_crypto)
    {
        throw LibLogicalAccessException("Cannot retrieve PKCSAESCrypto implementation");
    }
    return pkcs_crypto;
}
}
