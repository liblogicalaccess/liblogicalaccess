//
// Created by xaqq on 5/29/15.
//

#include "myclock.hpp"


MyClock &get_global_clock()
{
    static MyClock c;
    return c;
}
