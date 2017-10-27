#include <windows.h>
#include <sstream>

#include "logicalaccess/dynlibrary/winclass.hpp"

namespace logicalaccess
{
std::string WinClass::_getErrorMess(int errCode)
{
    char buffer[256];

    DWORD ret = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errCode, 0, buffer,
                                 256, nullptr);

    if (!ret)
    {
        std::stringstream ss;

        ss << "Unkown error: " << errCode << ".";
        return ss.str();
    }
    return std::string(buffer);
}
}