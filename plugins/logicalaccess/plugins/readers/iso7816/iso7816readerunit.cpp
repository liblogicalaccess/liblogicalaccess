/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief PC/SC reader unit.
 */

#include <iomanip>
#include <boost/filesystem.hpp>
#include <sstream>
#include <thread>

#include <logicalaccess/plugins/readers/iso7816/iso7816readerprovider.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunit.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>

#include <logicalaccess/plugins/cards/samav/samav2chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav1iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav2iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samiso7816resultchecker.hpp>
#include <boost/filesystem.hpp>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
ISO7816ReaderUnit::ISO7816ReaderUnit(std::string rpt)
    : ReaderUnit(rpt)
{
}

ISO7816ReaderUnit::~ISO7816ReaderUnit() {}

std::shared_ptr<Chip> ISO7816ReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> ret;
    return ret;
}

std::shared_ptr<ISO7816ReaderProvider> ISO7816ReaderUnit::getISO7816ReaderProvider() const
{
    return std::dynamic_pointer_cast<ISO7816ReaderProvider>(getReaderProvider());
}

std::vector<std::shared_ptr<Chip>> ISO7816ReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> ret;
    return ret;
}

std::shared_ptr<ResultChecker> ISO7816ReaderUnit::createDefaultResultChecker() const
{
    return std::make_shared<ISO7816ResultChecker>();
}

bool ISO7816ReaderUnit::reconnect(int /*action*/)
{
    try
    {
        auto chip = getSingleChip();
        if (chip && chip->getGenericCardType() == CHIP_SAM)
        {
            LOG(DEBUGS) << "Running specific behavior on reconnect with a connected SAM...";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            unlockSAM();
        }
    }
    catch (std::exception &ex)
    {
        LOG(ERRORS) << "ISO7816ReaderUnit reconnect: " << ex.what();
        return false;
    }

    return true;
}

void ISO7816ReaderUnit::connectToSAM()
{
    if (getISO7816Configuration()->getSAMType() != "SAM_NONE" &&
        getISO7816Configuration()->getSAMReaderName() == "" &&
        getISO7816Configuration()->getCheckSAMReaderIsAvailable())
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "SAM type specified without specifying SAM Reader Name");

    if (getISO7816Configuration()->getSAMType() != "SAM_NONE")
    {
        if (getISO7816Configuration()->getCheckSAMReaderIsAvailable())
        {
            if (getReaderProvider()->getReaderList().size() < 2)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "Not Enough reader on the system to use SAM");

            int i = 0;
            for (; i < static_cast<int>(getReaderProvider()->getReaderList().size()); ++i)
            {
                if (getReaderProvider()->getReaderList()[i]->getName() ==
                    getISO7816Configuration()->getSAMReaderName())
                    break;
            }

            if (i == (int)(getReaderProvider()->getReaderList().size()))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "The SAM Reader specified has not been find.");
        }

        std::shared_ptr<ISO7816ReaderUnit> ret;
        if (d_sam_readerunit &&
            (!getISO7816Configuration()->getCheckSAMReaderIsAvailable() ||
             d_sam_readerunit->getName() ==
                 getISO7816Configuration()->getSAMReaderName()))
        {
            ret = d_sam_readerunit;
        }
        else
        {
            ret = getISO7816ReaderProvider()->createReaderUnit(
                getISO7816Configuration()->getSAMReaderName());
        }

        if (getISO7816Configuration()->getAutoConnectToSAMReader())
        {
            ret->connectToReader();

            if (d_sam_client_context != "")
            {
                ret->setContext(d_sam_client_context);
            }
        }

        if (!ret->waitInsertion(1))
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "No SAM detected on the reader");

        ret->connect();
        LOG(LogLevel::INFOS) << "Checking SAM backward...";

        std::string SAMType = getISO7816Configuration()->getSAMType();
        if (SAMType != "SAM_AUTO" && SAMType != ret->getSingleChip()->getCardType())
            THROW_EXCEPTION_WITH_LOG(
                LibLogicalAccessException,
                "SAM on the reader is not the same type as selected.");

        LOG(LogLevel::INFOS) << "SAM backward ended.";

        setSAMChip(std::dynamic_pointer_cast<SAMChip>(ret->getSingleChip()));
        setSAMReaderUnit(ret);

        // If chip is se processor, we did enough. bail out not
        if (getISO7816Configuration()->getSAMType() == "SEProcessor")
        {
            // HID SE SAM. There is not more world to perform, the device
            // is ready-enough for us to use.

            // Maybe set_detected_card_info, not sure ? Maybe in future.
            return;
        }

        LOG(LogLevel::INFOS) << "Starting SAM Security check...";

        try
        {
            unlockSAM();
        }
        catch (std::exception &)
        {
            setSAMChip(std::shared_ptr<SAMChip>());
            setSAMReaderUnit(std::shared_ptr<ISO7816ReaderUnit>());
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "The SAM Detected is not the SAM waited.");
        }

        // Set SAM unlock key to the SAM Reader in case of reconnect
        ret->getISO7816Configuration()->setSAMUnlockKey(
            getISO7816Configuration()->getSAMUnLockKey(),
            getISO7816Configuration()->getSAMUnLockkeyNo());
    }
}

void ISO7816ReaderUnit::unlockSAM()
{
    if (!getISO7816Configuration()->getSAMUnLockKey())
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                         "The Unlock SAM key is empty.");

    auto samchip = getSAMChip();
    if (samchip->getCardType() == "SAM_AV1")
    {
        auto av1cmd = std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(samchip->getCommands());
        if (!av1cmd)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "No SAM AV1 commands.");
        }
        av1cmd->authenticateHost(getISO7816Configuration()->getSAMUnLockKey(),
                                getISO7816Configuration()->getSAMUnLockkeyNo());
    }
    else if (samchip->getCardType() == "SAM_AV2" || samchip->getCardType() == "SAM_AV3")
    {
        auto av2cmd = std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands());
        if (!av2cmd)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "No SAM AV2 commands.");
        }

        if (getISO7816Configuration()->getUseSAMAuthenticateHost())
        {
            av2cmd->authenticateHost(getISO7816Configuration()->getSAMUnLockKey(), getISO7816Configuration()->getSAMUnLockkeyNo());
        }
        else
        {
            try
            {
                av2cmd->lockUnlock(getISO7816Configuration()->getSAMUnLockKey(), Unlock,
                                getISO7816Configuration()->getSAMUnLockkeyNo(), 0,
                                0);
            }
            catch (CardException &ex)
            {
                if (ex.error_code() != CardException::WRONG_P1_P2)
                    std::rethrow_exception(std::current_exception());

                // try to lock the SAM in case it was already unlocked
                av2cmd->lockUnlock(getISO7816Configuration()->getSAMUnLockKey(),
                                LockWithoutSpecifyingKey,
                                getISO7816Configuration()->getSAMUnLockkeyNo(), 0,
                                0);

                av2cmd->lockUnlock(getISO7816Configuration()->getSAMUnLockKey(), Unlock,
                                getISO7816Configuration()->getSAMUnLockkeyNo(), 0,
                                0);
            }
        }
    }
}

bool ISO7816ReaderUnit::connectToReader()
{
    connectToSAM();
    return true;
}

void ISO7816ReaderUnit::disconnectFromSAM()
{
    if (getISO7816Configuration()->getSAMType() != "SAM_NONE" && d_sam_readerunit)
    {
        d_sam_readerunit->disconnect();
        if (getISO7816Configuration()->getAutoConnectToSAMReader())
        {
            d_sam_client_context = d_sam_readerunit->getContext();
            d_sam_readerunit->disconnectFromReader();
        }
        setSAMChip(std::shared_ptr<SAMChip>());
    }
}

void ISO7816ReaderUnit::disconnectFromReader()
{
    disconnectFromSAM();
}

void ISO7816ReaderUnit::reloadReaderConfiguration()
{
    LOG(LogLevel::INFOS) << "Starting reader configuration reloading...";
    // Run only SAM specific disconnect/connection
    ISO7816ReaderUnit::disconnectFromReader();
    ISO7816ReaderUnit::connectToReader();
    LOG(LogLevel::INFOS) << "Reader configuration completed.";
}

std::shared_ptr<SAMChip> ISO7816ReaderUnit::getSAMChip()
{
    return d_sam_chip;
}

void ISO7816ReaderUnit::setSAMChip(std::shared_ptr<SAMChip> t)
{
    d_sam_chip = t;
}

std::shared_ptr<ISO7816ReaderUnit> ISO7816ReaderUnit::getSAMReaderUnit()
{
    return d_sam_readerunit;
}

void ISO7816ReaderUnit::setSAMReaderUnit(std::shared_ptr<ISO7816ReaderUnit> t)
{
    d_sam_readerunit = t;
}

void ISO7816ReaderUnit::setContext(const std::string &context)
{
    d_client_context = context;
}

std::string ISO7816ReaderUnit::getContext()
{
    return d_client_context;
}
}
