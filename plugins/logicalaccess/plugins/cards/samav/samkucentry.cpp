/**
 * \file samkucentry.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief SAM KUC Entry source.
 */

#include <logicalaccess/plugins/cards/samav/samkucentry.hpp>

namespace logicalaccess
{
KucEntryUpdateSettings SAMKucEntry::getUpdateSettings()
{
    KucEntryUpdateSettings settings;

    bool *x = (bool *)&settings;
    for (unsigned char i = 0; i < sizeof(settings); ++i)
    {
        if ((d_updatemask & 0x80) == 0x80)
            x[i] = true;
        else
            x[i] = false;
        if (i + (unsigned int)1 < sizeof(settings))
            d_updatemask = d_updatemask << 1;
    }
    return settings;
}

void SAMKucEntry::setUpdateSettings(const KucEntryUpdateSettings &t)
{
    d_updatemask = SAMKucEntry::getUpdateMask(t);
}

unsigned char SAMKucEntry::getUpdateMask(const KucEntryUpdateSettings &t)
{
    bool *x      = (bool *)&t;
	unsigned char updatemask = 0;
    for (unsigned char i = 0; i < sizeof(t); ++i)
    {
        updatemask += x[i];
        if (i + (unsigned int)1 < sizeof(t))
            updatemask = updatemask << 1;
    }
	return updatemask;
}
}