#ifndef WINCLASS_H__
#define WINCLASS_H__

#include <string>

namespace LOGICALACCESS
{
	class WinClass
	{
	protected:
		static std::string _getErrorMess(int errCode);
	};
}

#endif