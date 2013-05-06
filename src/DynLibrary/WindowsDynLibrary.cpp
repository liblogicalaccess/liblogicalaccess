#include "logicalaccess/DynLibrary/WindowsDynLibrary.h"
#include "logicalaccess/logs.h"
#include "logicalaccess/ReaderProviders/ReaderUnit.h"

namespace LOGICALACCESS
{
	IDynLibrary* newDynLibrary(const std::string& dlName)
	{ return new WindowsDynLibrary(dlName); }

	WindowsDynLibrary::WindowsDynLibrary(const std::string& dlName)
		: _name(dlName)
	{
		if ((_handle = ::LoadLibrary(dlName.c_str())) == NULL)
		{
			ERROR_("Cannot load library %s.", dlName.c_str());
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, _getErrorMess(::GetLastError()));
		}
	}

	void* WindowsDynLibrary::getSymbol(const char* symName)
	{
		void* sym;
		std::string	err;

		sym = ::GetProcAddress(_handle, symName);
		if (!sym)
		{
			ERROR_("Cannot get symbol %s on library %s.", symName, _name.c_str());
			THROW_EXCEPTION_WITH_LOG(LibLOGICALACCESSException, _getErrorMess(::GetLastError()));
		}
		return sym;
	}
}
