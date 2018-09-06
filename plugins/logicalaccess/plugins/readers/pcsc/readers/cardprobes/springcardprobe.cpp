#include <logicalaccess/plugins/readers/pcsc/readers/cardprobes/springcardprobe.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>

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

        MifareAccessInfo::SectorAccessBits sab;
        auto ret = command->readSector(0, 1, std::shared_ptr<MifareKey>(),
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
        // If an error occurred, the card probably isn't desfire.
        return false;
    }
    return false;
}
