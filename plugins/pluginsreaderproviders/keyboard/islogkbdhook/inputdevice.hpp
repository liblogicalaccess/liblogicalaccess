#ifndef _DEFINED_INPUTDEVICE
#define _DEFINED_INPUTDEVICE

#include "stdafx.h"
#include <vector>

#include "../keyboardsharedstruct.hpp"

namespace KBDHOOK
{
class InputDevice
{
  public:
    static std::vector<logicalaccess::KeyboardEntry> getDeviceList();
};
}

#endif
