/**
 * \file test_brut_force.cpp
 * \author Leo Jullerot
 * \brief Brut force DESFireEV1 cards (Applications IDs, Files IDs, Keys numbers)
 * \information BrutForce time with NFC 'ROLL : 0,1,2 and 3 unknown mask value : < 1 min
 *                                              4 unknown mask value : between 4 min and 5 min
 *                                              5 unknown mask value : 1 hour 15 min
 *                                              6 unknown mask value : 18 hours
 */

#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/readerproviders/serialportdatatransport.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev1chip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireev1iso7816commands.hpp>
#include <logicalaccess/plugins/lla-tests/macros.hpp>
#include <logicalaccess/plugins/lla-tests/utils.hpp>
#include <logicalaccess/utils.hpp>
#include "test_appIDs_brutForce_class.h"

void introduction()
{
    PRINT_TIME("This test return applications IDs list of DESFireEV1 cards protecteds.");

    PRINT_TIME("You will have 20 seconds to insert a card. Test log below");
    PRINT_TIME("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    LLA_SUBTEST_REGISTER("Reader connection");
    LLA_SUBTEST_REGISTER("SelectedOption");
    LLA_SUBTEST_REGISTER("BrutForceCard");
}

std::vector<AppIDsBrutForce> brutForceAppIDs(
        std::shared_ptr<logicalaccess::DESFireISO7816Commands> &cmd, std::string &mask)
{
    int loop = 0, nbError = 0, nbvalue = 1;
    bool notOnMask;
    uint64_t lowerValue, upperValue;
    std::string lowerMask, upperMask;
    uint64_t oldI = 0;
    std::vector<AppIDsBrutForce> appIDsUsing;

    //calculation of upper and lower value for brutForce
    for (int i = 0; i < 6; i++)
    {
        if (mask[i] == '_')
        {
            lowerMask[i] = '0';
            upperMask[i] = 'f';
            nbvalue *= 16;
        }
        else
        {
            lowerMask[i] = mask[i];
            upperMask[i] = mask[i];
        }
    }

    lowerValue = static_cast<uint64_t>(std::stoi(lowerMask, nullptr,16));
    upperValue = static_cast<uint64_t>(std::stoi(upperMask,nullptr,16));

    // print percentage of progression
    for (uint64_t i = lowerValue; i <= upperValue; i++)
    {
        if (nbvalue > 2000000) { //0.01% print
            if ((i - lowerValue) % ((upperValue - lowerValue) / 10000) == 0 && i != 0)
            {
                PRINT_TIME("[" << ((i - lowerValue) * 10000 / (upperValue - lowerValue)) / 100.0 << "%]");
            }
        } else if (nbvalue > 100000) { //0.1% print
            if ((i - lowerValue) % ((upperValue - lowerValue) / 1000) == 0 && i != 0)
            {
                PRINT_TIME("[" << ((i - lowerValue) * 1000 / (upperValue - lowerValue)) / 10.0 << "%]");
            }
        } else if (nbvalue > 1000) { //1% print
            if ((i - lowerValue) % ((upperValue - lowerValue) / 100) == 0 && i != 0)
            {
                PRINT_TIME("[" << (i - lowerValue) * 100 / (upperValue - lowerValue) << "%]");
            }
        }

        if (i == upperValue)
        {
            PRINT_TIME("[100%]");
            LLA_SUBTEST_PASSED("BrutForceCard");
        }

        std::stringstream sstream;
        sstream << std::hex << i;
        std::string result = sstream.str();

        notOnMask = false;
        for (unsigned int resultSize = result.size(); resultSize < 6; resultSize ++)
        {
            result = "0" + result;
        }

        for (unsigned int j = 0; j < 6; j++)
        {
            if ((result[j] != mask[j]) && (mask[j] != '_'))
            {
                notOnMask = true;
            }
        }

        if (!notOnMask) {
            try
            {
                cmd->selectApplication(static_cast<unsigned int>(i));
                AppIDsBrutForce application(cmd, static_cast<unsigned int>(i));

                application.brutForceFileIDs();
                application.brutForceKeyNo();

                appIDsUsing.push_back(application);
            }
            catch (logicalaccess::CardException &ex)
            {
                if (ex.error_code() == logicalaccess::CardException::AID_NOT_FOUND)
                {
                    continue;
                }
                else
                {
                    nbError++;
                    if (oldI == i)
                    {
                        loop++;
                    }
                    else
                    {
                        loop = 0;
                        oldI = i;
                    }
                    if (loop == 100)
                    {
                        std::cout << ex.what() << std::endl;
                        break;
                    }
                    i--;
                }
            }
        }
    }
    return appIDsUsing;
}

int main(int ac, char **av)
{
    prologue(ac, av);
    introduction();
    logicalaccess::ReaderProviderPtr provider;
    logicalaccess::ReaderUnitPtr readerUnit;
    logicalaccess::ChipPtr chip;
    tie(provider, readerUnit, chip) = lla_test_init("DESFireEV1");

    PRINT_TIME("CHip identifier: "
                       << logicalaccess::BufferHelper::getHex(chip->getChipIdentifier()));

    auto cmd = std::dynamic_pointer_cast<logicalaccess::DESFireISO7816Commands>(chip->getCommands());
    unsigned char maskOption;

    LLA_SUBTEST_PASSED("Reader connection");

    logicalaccess::ElapsedTimeCounter etc;

    std::cout << "Do you want to apply a mask on this brutForce test? [y/n]" << std::endl;
    std::cin >> maskOption;

    std::string mask;

    if (toupper(maskOption) == 'Y')
    {
        while (mask.empty()) {
            std::cout << "Entry your mask in the format _f_f__ : ";
            std::cin >> mask;

            if (mask.size() != 6)
            {
                std::cout << "Invalid size mask" << std::endl;
                mask = "";
            }
            else
            {
                for (unsigned int i = 0; i != 6; i++) {
                    if (!((mask[i] >= '0' && mask[i] <= '9')
                          || (mask[i] >= 'a' && mask[i] <= 'f')
                          || (mask[i] >= 'A' && mask[i] <= 'F')
                          || mask[i] == '_'))
                    {
                        std::cout << "Invalid value on mask" << std::endl;
                        mask = "";
                        break;
                    }
                }
            }
        }
        std::cout << "Mask used: " << mask << std::endl;
    }
    else
    {
        mask = "______";
    }
    LLA_SUBTEST_PASSED("SelectedOption");

    std::vector<AppIDsBrutForce> appIDsUsing = brutForceAppIDs(cmd, mask);

    for (AppIDsBrutForce &it : appIDsUsing)
    {
        std::cout << std::endl << "\t" << "Reading the application 0x" << std::hex << it.getAppID()
                  << " with key type "
        << it.getKeyType() << ":"<< std::endl;
        std::cout << "\t" << "\t" << "List of Files IDs:"<< std::endl;

        for(unsigned int iterator : it.getFilesIDs())
        {
            std::cout << "\t" << "\t" << "\t 0x" << std::hex << iterator << std::endl;
        }
        std::cout << "\t" << "\t" << "List of numbers key:"<< std::endl;

        for(unsigned int iterator : it.getKeysNo())
        {
            std::cout << "\t" << "\t" << "\t 0x" << std::hex << iterator << std::endl;
        }
    }

    auto hours = (int)(etc.elapsed()/3600000);
    auto min = (int)((etc.elapsed() - hours * 3600000) / 60000);
    auto sec = (int)((etc.elapsed() - hours * 3600000 - min * 60000)/1000);
    auto milli = (int)((etc.elapsed() - hours * 3600000 - min * 60000 - sec * 1000));

    std::cout << std::endl << "Execution time: " << std::dec << hours << " hour(s) " << min
              << " minute(s) " << sec << " second(s) " << milli << " millisecond(s)." << std::endl;

    return EXIT_SUCCESS;
}