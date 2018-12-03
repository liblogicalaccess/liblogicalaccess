#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/generictag/generictagchip.hpp>
#include <logicalaccess/plugins/cards/generictag/lla_cards_generictag_api.hpp>

extern "C" {
LLA_CARDS_GENERICTAG_API char *getLibraryName()
{
    return (char *)CHIP_GENERICTAG;
}

LLA_CARDS_GENERICTAG_API void
getGenericTagChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>();
    }
}

LLA_CARDS_GENERICTAG_API void
getGENERIC_T_CLChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>("GENERIC_T_CL");
    }
}

LLA_CARDS_GENERICTAG_API void
getGENERIC_T_CL_AChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::GenericTagChip>("GENERIC_T_CL_A");
    }
}

LLA_CARDS_GENERICTAG_API void
setTagIdBitsLengthOfGenericTagChip(std::shared_ptr<logicalaccess::Chip> *chip,
                                   unsigned int bits)
{
    if (chip != nullptr)
    {
        std::dynamic_pointer_cast<logicalaccess::GenericTagChip>(*chip)
            ->setTagIdBitsLength(bits);
    }
}

LLA_CARDS_GENERICTAG_API bool getChipInfoAt(unsigned int index, char *chipname,
                                            size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getGenericTagChip;
            sprintf(chipname, CHIP_GENERICTAG);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}