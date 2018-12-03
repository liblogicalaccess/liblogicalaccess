#include <string>
#include <memory>
#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/cards/mifare/mifare1kchip.hpp>
#include <logicalaccess/plugins/cards/mifare/mifare4kchip.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

extern "C" {
LLA_CARDS_MIFARE_API char *getLibraryName()
{
    return (char *)"Mifare";
}

LLA_CARDS_MIFARE_API void getMifare1KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::Mifare1KChip>();
    }
}

LLA_CARDS_MIFARE_API void getMifare4KChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::Mifare4KChip>();
    }
}

LLA_CARDS_MIFARE_API void getMifareChip(std::shared_ptr<logicalaccess::Chip> *chip)
{
    if (chip != nullptr)
    {
        *chip = std::make_shared<logicalaccess::MifareChip>();
    }
}

LLA_CARDS_MIFARE_API bool getChipInfoAt(unsigned int index, char *chipname,
                                        size_t chipnamelen, void **getterfct)
{
    bool ret = false;
    if (chipname != nullptr && chipnamelen == PLUGINOBJECT_MAXLEN && getterfct != nullptr)
    {
        switch (index)
        {
        case 0:
        {
            *getterfct = (void *)&getMifare1KChip;
            sprintf(chipname, CHIP_MIFARE1K);
            ret = true;
        }
        break;
        case 1:
        {
            *getterfct = (void *)&getMifare1KChip;
            sprintf(chipname, CHIP_MIFARE4K);
            ret = true;
        }
        break;
        default:;
        }
    }

    return ret;
}
}