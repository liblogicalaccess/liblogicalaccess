#include "logicalaccess/myexception.hpp"
#include "logicalaccess/dynlibrary/windowsdynlibrary.hpp"
#include "logicalaccess/logs.hpp"
#include "logicalaccess/readerproviders/readerunit.hpp"

namespace logicalaccess
{
	IDynLibrary* newDynLibrary(const std::string& dlName)
	{ return new WindowsDynLibrary(dlName); }

	WindowsDynLibrary::WindowsDynLibrary(const std::string& dlName)
		: _name(dlName)
	{
		if ((_handle = ::LoadLibrary(dlName.c_str())) == NULL)
		{
			LOG(LogLevel::PLUGINS_ERROR) << "Cannot load library %s.", dlName.c_str());
			throw EXCEPTION(LibLogicalAccessException, _getErrorMess(::GetLastError()));
		}
	}

	void* WindowsDynLibrary::getSymbol(const char* symName)
	{
		void* sym;

		sym = ::GetProcAddress(_handle, symName);
		if (!sym)
		{
			LOG(LogLevel::PLUGINS_ERROR) << "Cannot get symbol %s on library %s.", symName, _name.c_str());
			throw EXCEPTION(LibLogicalAccessException, _getErrorMess(::GetLastError()));
		}
		return sym;
	}
}
