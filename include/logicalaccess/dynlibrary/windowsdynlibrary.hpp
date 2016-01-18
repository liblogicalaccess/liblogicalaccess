#ifndef WINDOWSDYNLIBRARY_HPP__
#define WINDOWSDYNLIBRARY_HPP__

#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <string>

#include "logicalaccess/dynlibrary/idynlibrary.hpp"
#include "logicalaccess/dynlibrary/winclass.hpp"

namespace logicalaccess
{
    class WindowsDynLibrary : public IDynLibrary, private WinClass
    {
    private:
        std::string _name;
        HMODULE _handle;

    public:
        WindowsDynLibrary(const std::string& dlName);
		~WindowsDynLibrary();

        const std::string& getName(void) const
        {
            return _name;
        }

        void* getSymbol(const char* symName);

        virtual bool hasSymbol(const char *name);
    };
}

#endif