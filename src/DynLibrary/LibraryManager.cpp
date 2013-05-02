#include "logicalaccess/DynLibrary/LibraryManager.h"
#include <boost/filesystem.hpp>

namespace LOGICALACCESS
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
		boost::filesystem::path pluginDir(boost::filesystem::current_path());
		boost::filesystem::directory_iterator end_iter;
		std::string extension = EXTENSION_LIB;


		for (std::map<std::string, IDynLibrary*>::iterator it = libLoaded.begin(); it != libLoaded.end(); ++it)
		{
			try
			{
				if ((*it).second != NULL && (fct = (*it).second->getSymbol(fctname.c_str())) != NULL)
					return fct;
			}
			catch (...) {}
		}


		if (boost::filesystem::exists(pluginDir) && boost::filesystem::is_directory(pluginDir))
		{
			for (boost::filesystem::directory_iterator dir_iter(pluginDir) ; dir_iter != end_iter ; ++dir_iter)
			{
				if (boost::filesystem::is_regular_file(dir_iter->status())
					&& dir_iter->path().extension() == extension && libLoaded[dir_iter->path().filename().string()] == NULL
					&& (hasEnding(dir_iter->path().filename().string(), enumType[libraryType] + extension)
					|| hasEnding(dir_iter->path().filename().string(), "Unified" + extension)))
				{
					try
					{
						IDynLibrary* lib = newDynLibrary(dir_iter->path().string());
						fct = lib->getSymbol(fctname.c_str());
						if (fct != NULL)
						{
							libLoaded[dir_iter->path().filename().string()] = lib;
							return fct;
						}
						else
							delete lib;
					}
					catch (...) {}
				}
			}
		}
		else
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, "No Plugins Folder found");
		return NULL;
	}

	boost::shared_ptr<ReaderProvider> LibraryManager::getReaderProvider(const std::string& readertype)
	{
		boost::shared_ptr<ReaderProvider> ret;
		std::string fctname = "get" + readertype + "Reader";

		getprovider getpcscsfct;
		*(void**)(&getpcscsfct) = getFctFromName(fctname, LibraryManager::READERS_TYPE);
		if (!getpcscsfct)
		{
			*(void**)(&getpcscsfct) = getFctFromName(fctname, LibraryManager::UNIFIED_TYPE);
		}

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
		if (!getcardfct)
		{
			*(void**)(&getcardfct) = getFctFromName(fctname, LibraryManager::UNIFIED_TYPE);
		}

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
		if (!getcardproviderfct)
		{
			*(void**)(&getcardproviderfct) = getFctFromName(fctname, LibraryManager::UNIFIED_TYPE);
		}

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
		if (!getcommandsfct)
		{
			*(void**)(&getcommandsfct) = getFctFromName(fctname, LibraryManager::UNIFIED_TYPE);
		}

		if (getcommandsfct)
		{
			getcommandsfct(&ret);
		}

		return ret;
	}
}