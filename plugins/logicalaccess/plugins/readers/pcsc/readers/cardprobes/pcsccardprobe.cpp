#include <logicalaccess/plugins/readers/pcsc/readers/cardprobes/pcsccardprobe.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <assert.h>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightccommands.hpp>

using namespace logicalaccess;

PCSCCardProbe::PCSCCardProbe(ReaderUnit *ru)
    : CardProbe(ru)
{
}

bool PCSCCardProbe::maybe_mifare_classic()
{
    try
    {
        LLA_LOG_CTX("Probe::maybe_mifare_classic");
        reset();
        auto chip = reader_unit_->createChip("Mifare1K");
        std::shared_ptr<MifareCommands> command =
            std::dynamic_pointer_cast<MifareCommands>(chip->getCommands());

        MifareAccessInfo::SectorAccessBits sab;
        auto ret = command->readSector(1, 0, std::shared_ptr<MifareKey>(),
                                       std::shared_ptr<MifareKey>(), sab);
        return true;
    }
    catch (const CardException &e)
    {
        if (e.error_code() == CardException::ErrorType::SECURITY_STATUS)
        {
            // Permission error with this reader may means the card is a Mifare
            // Classic.
            return true;
        }
    }
    catch (const std::exception &)
    {
        // If an error occurred, the card probably isn't mifare classic.
        return false;
    }
    return false;
}

bool PCSCCardProbe::is_desfire(std::vector<uint8_t> *uid)
{
    try
    {
        LLA_LOG_CTX("Probe::is_desfire");
        reset();
        auto chip = reader_unit_->createChip("DESFire");
        auto desfire_command =
            std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
        desfire_command->selectApplication(0x00);
        DESFireCommands::DESFireCardVersion cardversion = desfire_command->getVersion();

        if (uid)
            *uid = ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
        return true;
    }
    catch (const std::exception &)
    {
        // If an error occurred, the card probably isn't desfire.
        return false;
    }
}

int PCSCCardProbe::get_desfire_version(std::vector<uint8_t> *uid)
{
    int try_count = 0;
    while (true)
    {
        try
        {
            LLA_LOG_CTX("Probe::get_desfire_version");
            reset();
            auto chip = reader_unit_->createChip("DESFireEV1");
            auto desfire_command =
                std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
            assert(desfire_command);
            desfire_command->selectApplication(0x00);
            DESFireCommands::DESFireCardVersion cardversion =
                desfire_command->getVersion();

            if (uid)
                *uid = ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
            return cardversion.softwareMjVersion;
        }
        catch (const std::exception &)
        {
            // Quite often the hardware is not ready or something else happens
            // which cause the command to fail. So we try a few time.
            // This seems to fix somewhat reliably the Desfire / EV1 / EV2 detection
            // issues.
            if (try_count++ == 2)
            {
                // If an error occurred, the card probably isn't desfire.
                return -1;
            }
        }
    }
}

bool PCSCCardProbe::is_desfire_ev1(std::vector<uint8_t> *uid)
{
    LLA_LOG_CTX("Probe::is_desfire_ev1");
    return get_desfire_version(uid) == 1;
}

bool PCSCCardProbe::is_desfire_ev2(std::vector<uint8_t> *uid)
{
    LLA_LOG_CTX("Probe::is_desfire_ev2");
    return get_desfire_version(uid) >= 2;
}

bool PCSCCardProbe::is_mifare_ultralight_c()
{
    try
    {
        LLA_LOG_CTX("Probe::is_mifare_ultralight_c");
        reset();
        auto chip = reader_unit_->createChip("MifareUltralightC");
        auto mfu_command =
            std::dynamic_pointer_cast<MifareUltralightCCommands>(chip->getCommands());
        assert(mfu_command);
        mfu_command->authenticate(std::shared_ptr<TripleDESKey>());
    }
    catch (const std::exception &)
    {
        // TODO: handle the case authentication is not default by checking error code
        return false;
    }

    return true;
}


void PCSCCardProbe::reset() const
{
    try
    {
        auto pcsc_ru = dynamic_cast<PCSCReaderUnit *>(reader_unit_);
        assert(pcsc_ru);

        pcsc_ru->reset(SCARD_UNPOWER_CARD);
    }
    catch (const std::exception &e)
    {
        LOG(ERRORS) << "PCSCCardProbe reset failure: " << e.what();
        THROW_EXCEPTION_WITH_LOG(
            LibLogicalAccessException,
            "Card probing failed because reseting the PCSC connection failed.");
    }
}

bool PCSCCardProbe::has_desfire_random_uid(ByteVector *uid)
{
    try
    {
        auto chip = reader_unit_->createChip("DESFireEV1");
        auto desfire_command =
            std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
        assert(desfire_command);

        desfire_command->selectApplication(0x00);
        DESFireCommands::DESFireCardVersion cardversion = desfire_command->getVersion();

        if (BufferHelper::allZeroes(cardversion.uid))
        {
            return true;
        }
        if (uid)
            *uid = ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
        return false;
    }
    catch (const std::exception &)
    {
        return false;
    }
}