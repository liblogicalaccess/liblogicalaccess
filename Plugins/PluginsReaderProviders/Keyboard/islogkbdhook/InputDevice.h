#ifndef _DEFINED_INPUTDEVICE
#define _DEFINED_INPUTDEVICE

#include "stdafx.h"
#include <vector>

#include "../KeyboardSharedStruct.h"

namespace KBDHOOK
{
	class InputDevice
	{
		public:

			static std::vector<LOGICALACCESS::KeyboardEntry> getDeviceList();
	};
}

#endif