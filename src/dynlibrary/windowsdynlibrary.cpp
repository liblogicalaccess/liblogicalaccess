#include <boost/asio.hpp>
#include "logicalaccess/dynlibrary/windowsdynlibrary.hpp"
#include "logicalaccess/myexception.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/readerproviders/readerunit.hpp"

namespace logicalaccess
{
    IDynLibrary* newDynLibrary(const std::string& dlName)
    {
        return new WindowsDynLibrary(dlName);
    }

    WindowsDynLibrary::WindowsDynLibrary(const std::string& dlName)
        : _name(dlName)
    {
        if ((_handle = ::LoadLibrary(dlName.c_str())) == NULL)
        {
            LOG(LogLevel::PLUGINS_ERROR) << "Cannot load library " << dlName << ".";
            throw EXCEPTION(LibLogicalAccessException, _getErrorMess(::GetLastError()));
        }
    }

	WindowsDynLibrary::~WindowsDynLibrary()
	{
		::FreeLibrary(_handle);
	}

    void* WindowsDynLibrary::getSymbol(const char* symName)
    {
        void* sym;

        sym = ::GetProcAddress(_handle, symName);
        if (!sym)
        {
            LOG(LogLevel::PLUGINS_ERROR) << "Cannot get symbol " << symName << " on library " << _name << ".";
            throw EXCEPTION(LibLogicalAccessException, _getErrorMess(::GetLastError()));
        }
        return sym;
    }

    bool WindowsDynLibrary::hasSymbol(const char *name)
    {
        void* sym;

        sym = ::GetProcAddress(_handle, name);
        if (!sym)
        {
            return false;
        }
        return true;
    }
}