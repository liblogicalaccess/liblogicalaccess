#ifndef WINCLASS_HPP__
#define WINCLASS_HPP__

#include <string>

namespace logicalaccess
{
    class WinClass
    {
    protected:
        static std::string _getErrorMess(int errCode);
    };
}

#endif