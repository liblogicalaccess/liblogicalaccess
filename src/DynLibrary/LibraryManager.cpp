#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include <boost/filesystem.hpp>

namespace logicalaccess
{
	const std::string LibraryManager::enumType[3] = {"Readers", "Cards", "Unified"};

	bool LibraryManager::hasEnding(std::string const &fullString, std::string ending)
	{
		if (fullString.length() >= ending.length()) {
			return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
		} else {
			return false;
		}
	}

	LIBLOGICALACCESS_API LibraryManager *LibraryManager::_singleton = NULL;

	void* LibraryManager::getFctFromName(std::string fctname, LibraryType libraryType)
	{
		void *fct;
		boost::filesystem::directory_iterator end_iter;
		std::string extension = EXTENSION_LIB;
		Settings setting = Settings::getInstance();

		if (libLoaded.size() == 0)
		{
			scanPlugins();
		}

		for (std::map<std::string, IDynLibrary*>::iterator it = libLoaded.begin(); it != libLoaded.end(); ++it)
		{
			try
			{
				if ((*it).second != NULL && (fct = (*it).second->getSymbol(fctname.c_str())) != NULL)
					return fct;
			}
			catch (...) {}
		}

		return NULL;
	}

	boost::shared_ptr<ReaderProvider> LibraryManager::getReaderProvider(const std::string& readertype)
	{
		boost::shared_ptr<ReaderProvider> ret;
		std::string fctname = "get" + readertype + "Reader";

		getprovider getpcscsfct;
		*(void**)(&getpcscsfct) = getFctFromName(fctname, LibraryManager::READERS_TYPE);

		if (getpcscsfct)
		{
			getpcscsfct(&ret);
		}

		return ret;
	}

	boost::shared_ptr<Chip> LibraryManager::getCard(const std::string& cardtype)
	{
		boost::shared_ptr<Chip> ret;
		std::string fctname = "get" + cardtype + "Chip";

		getcard getcardfct;
		*(void**)(&getcardfct) = getFctFromName(fctname, LibraryManager::CARDS_TYPE);

		if (getcardfct)
		{
			getcardfct(&ret);
		}

		return ret;
	}

	boost::shared_ptr<CardProvider> LibraryManager::getCardProvider(const std::string& cardtype)
	{
		boost::shared_ptr<CardProvider> ret;
		std::string fctname = "get" + cardtype + "CardProvider";

		getcardprovider getcardproviderfct;
		*(void**)(&getcardproviderfct) = getFctFromName(fctname, LibraryManager::CARDS_TYPE);
		if (getcardproviderfct)
		{
			getcardproviderfct(&ret);
		}

		return ret;
	}

	boost::shared_ptr<Commands> LibraryManager::getCommands(const std::string& extendedtype)
	{
		boost::shared_ptr<Commands> ret;
		std::string fctname = "get" + extendedtype + "Commands";

		getcommands getcommandsfct;
		*(void**)(&getcommandsfct) = getFctFromName(fctname, LibraryManager::READERS_TYPE);
		if (getcommandsfct)
		{
			getcommandsfct(&ret);
		}

		return ret;
	}

	std::vector<std::string> LibraryManager::getAvailableCards()
	{
		return getAvailablePlugins(LibraryManager::CARDS_TYPE);
	}

	std::vector<std::string> LibraryManager::getAvailableReaders()
	{
		return getAvailablePlugins(LibraryManager::READERS_TYPE);
	}

	std::vector<std::string> LibraryManager::getAvailablePlugins(LibraryType libraryType)
	{
		std::vector<std::string> plugins;
		void* fct = NULL;
		boost::filesystem::directory_iterator end_iter;
		std::string extension = EXTENSION_LIB;
		Settings setting = Settings::getInstance();
		std::string fctname;
		
		if (libraryType == LibraryManager::CARDS_TYPE)
		{
			fctname = "getChipInfoAt";
		}
		else if (libraryType == LibraryManager::READERS_TYPE)
		{
			fctname = "getReaderInfoAt";
		}	

		if (libLoaded.size() == 0)
		{
			scanPlugins();
		}

		for (std::map<std::string, IDynLibrary*>::iterator it = libLoaded.begin(); it != libLoaded.end(); ++it)
		{
			try
			{
				if ((*it).second != NULL && (fct = (*it).second->getSymbol(fctname.c_str())) != NULL)
				{
					getobjectinfoat objectinfoptr;
					*(void**)(&objectinfoptr) = fct;
					getAvailablePlugins(plugins, objectinfoptr);
				}
			}
			catch (...) {}
		}

		return plugins;
	}

	void LibraryManager::getAvailablePlugins(std::vector<std::string>& plugins, getobjectinfoat objectinfoptr)
	{
		char objectname[PLUGINOBJECT_MAXLEN];
		memset(objectname, 0x00, sizeof(objectname));
		void* getobjectptr;

		unsigned int i = 0;
		while(objectinfoptr(i, objectname, sizeof(objectname), &getobjectptr))
		{
			plugins.push_back(std::string(objectname));
			memset(objectname, 0x00, sizeof(objectname));
			++i;
		}
	}

	void LibraryManager::scanPlugins()
	{
		void* fct = NULL;
		boost::filesystem::directory_iterator end_iter;
		std::string extension = EXTENSION_LIB;
		Settings setting = Settings::getInstance();
		std::string fctname = "getLibraryName";

		for (std::vector<std::string>::iterator it = setting.PluginFolders.begin(); it != setting.PluginFolders.end(); ++it)
		{
			boost::filesystem::path pluginDir(*it);
			if (boost::filesystem::exists(pluginDir) && boost::filesystem::is_directory(pluginDir))
			{
				PLUGIN_("Scanning library folder %s...", pluginDir.string().c_str());
				for (boost::filesystem::directory_iterator dir_iter(pluginDir) ; dir_iter != end_iter ; ++dir_iter)
				{
					PLUGIN_("Checking library %s...", dir_iter->path().filename().string().c_str());
					if (boost::filesystem::is_regular_file(dir_iter->status())
						&& dir_iter->path().extension() == extension
						&& (hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::CARDS_TYPE] + extension)
						|| hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::READERS_TYPE] + extension)
						|| hasEnding(dir_iter->path().filename().string(), enumType[LibraryManager::UNIFIED_TYPE] + extension)))
					{
						try
						{
							if (libLoaded.find(dir_iter->path().filename().string()) == libLoaded.end())
							{
								IDynLibrary* lib = newDynLibrary(dir_iter->path().string());
								fct = lib->getSymbol(fctname.c_str());
								if (fct != NULL)
								{
									PLUGIN_("Library %s loaded.", dir_iter->path().filename().string().c_str());
									libLoaded[dir_iter->path().filename().string()] = lib;
								}
								else
								{
									PLUGIN_("Cannot found library entry point in %s. Skipped.", dir_iter->path().filename().string().c_str());
									delete lib;
								}
							}
							else
							{
								PLUGIN_("Library %s already loaded. Skipped.", dir_iter->path().filename().string().c_str());
							}
						}
						catch (...) {}
					}
					else
					{
						PLUGIN_("File %s does not match excepted filenames. Skipped.", dir_iter->path().filename().string().c_str());
					}
				}
			}
			else
			{
				WARNING_("Cannot found plug-in folder %s", (*it).c_str());
			}
		}
	}
}