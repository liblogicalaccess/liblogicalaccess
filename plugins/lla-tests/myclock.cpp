//
// Created by xaqq on 5/29/15.
//

#include "logicalaccess/tests/tools/myclock.hpp"


MyClock &get_global_clock()
{
	static MyClock c;
	return c;
}
