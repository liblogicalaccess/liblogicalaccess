#ifndef WINDOWSDYNLIBRARY_H__
#define WINDOWSDYNLIBRARY_H__

#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <string>

#include "logicalaccess/DynLibrary/IDynLibrary.h"
#include "logicalaccess/DynLibrary/WinClass.h"

namespace logicalaccess
{
	class WindowsDynLibrary : public IDynLibrary, private WinClass
	{
	private:
		std::string _name;
		HMODULE _handle;

	public:
		WindowsDynLibrary(const std::string& dlName);
		~WindowsDynLibrary()
		{ ::FreeLibrary(_handle); }

		const std::string& getName(void) const
		{ return _name; }
		void* getSymbol(const char* symName);
	};
}

#endif