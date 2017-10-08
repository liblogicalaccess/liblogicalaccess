#include <string>
#include <memory>
#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "desfireev1chip.hpp"
#include "desfirechip.hpp"
#include "sagemkeydiversification.hpp"
#include "nxpav2keydiversification.hpp"
#include "nxpav1keydiversification.hpp"
#include "omnitechkeydiversification.hpp"
#include "logicalaccess/cards/keydiversification.hpp"

#ifdef _MSC_VER
#include "logicalaccess/msliblogicalaccess.h"
#else
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#ifndef DISABLE_PRAGMA_WARNING
#define DISABLE_PRAGMA_WARNING /**< \brief winsmcrd.h was modified to support this macro, to avoid MSVC specific warnings pragma */
#endif
#endif

extern "C"
{
    LIBLOGICALACCESS_API char *getLibraryName()
    {
        return (char *)"DESFire";
    }

    LIBLOGICALACCESS_API void getDESFireEV1Chip(std::shared_ptr<logicalaccess::Chip>* chip)
    {
        if (chip != nullptr)
        {
            *chip = std::make_shared<logicalaccess::DESFireEV1Chip>();
        }
    }

    LIBLOGICALACCESS_API void getDESFireChip(std::shared_ptr<logicalaccess::Chip>* chip)
    {
        if (chip != nullptr)
        {
            *chip = std::make_shared<logicalaccess::DESFireChip>();
        }
    }

    LIBLOGICALACCESS_API void getNXPAV1Diversification(std::shared_ptr<logicalaccess::KeyDiversification>* keydiversification)
    {
        if (keydiversification != nullptr)
        {
            *keydiversification = std::make_shared<logicalaccess::NXPAV1KeyDiversification>();
        }
    }

    LIBLOGICALACCESS_API void getNXPAV2Diversification(std::shared_ptr<logicalaccess::KeyDiversification>* keydiversification)
    {
        if (keydiversification != nullptr)
        {
            *keydiversification = std::make_shared<logicalaccess::NXPAV2KeyDiversification>();
        }
    }

    LIBLOGICALACCESS_API void getSagemDiversification(std::shared_ptr<logicalaccess::KeyDiversification>* keydiversification)
    {
        if (keydiversification != nullptr)
        {
            *keydiversification = std::make_shared<logicalaccess::SagemKeyDiversification>();
        }
    }

	LIBLOGICALACCESS_API void getOmnitechDiversification(std::shared_ptr<logicalaccess::KeyDiversification>* keydiversification)
    {
        if (keydiversification != nullptr)
        {
            *keydiversification = std::make_shared<logicalaccess::OmnitechKeyDiversification>();
        }
    }

    LIBLOGICALACCESS_API bool getChipInfoAt(unsigned int index, char* chipname, size_t chipnamelen, void** getterfct)
    {
        bool ret = false;
        if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
        {
            switch (index)
            {
            case 0:
            {
                *getterfct = (void*)&getDESFireChip;
                sprintf(chipname, CHIP_DESFIRE);
                ret = true;
            }
                break;

            case 1:
            {
                *getterfct = (void*)&getDESFireEV1Chip;
                sprintf(chipname, CHIP_DESFIRE_EV1);
                ret = true;
            }
                break;
            default: ;
            }
        }

        return ret;
    }
}