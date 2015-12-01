#include "logicalaccess/logs.hpp"
#include "mifarecommands.hpp"
#include "mifareprofile.hpp"
#include "springcardprobe.hpp"
#include "logicalaccess/cards/chip.hpp"

using namespace logicalaccess;

SpringCardProbe::SpringCardProbe(ReaderUnit *ru)
    : PCSCCardProbe(ru)
{
}

bool SpringCardProbe::maybe_mifare_classic()
{
    try
    {
        LLA_LOG_CTX("Probe::maybe_mifare_classic");
        reset();
        auto chip = reader_unit_->createChip("Mifare1K");
        std::shared_ptr<MifareCommands> command =
            std::dynamic_pointer_cast<MifareCommands>(chip->getCommands());

        logicalaccess::MifareAccessInfo::SectorAccessBits sab;
        std::dynamic_pointer_cast<logicalaccess::MifareProfile>(chip->getProfile())
            ->setDefaultKeysAt(0x00);
        auto ret = command->readSector(0, 1, sab);
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
        // If an error occurred, the card probably isn't desfire.
        return false;
    }
    return false;
}
