#include <windows.h>
#include <sstream>

#include "logicalaccess/DynLibrary/WinClass.h"

namespace logicalaccess
{
	std::string WinClass::_getErrorMess(int errCode)
	{
		DWORD ret;
		char buffer[256];

		ret = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errCode, 0, buffer, 256, NULL);

		if (!ret)
		{
			std::stringstream ss;

			ss << "Unkown error: " << errCode << ".";
			return ss.str();
		}
		return std::string(buffer);
	}
}