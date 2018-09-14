#include <logicalaccess/plugins/readers/pcsc/readers/cardprobes/cl1356cardprobe.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarechip.hpp>

using namespace logicalaccess;

CL1356CardProbe::CL1356CardProbe(ReaderUnit *ru)
    : PCSCCardProbe(ru)
{
}

bool CL1356CardProbe::maybe_mifare_classic()
{
    try
    {
        LLA_LOG_CTX("CL1356CardProbe::maybe_mifare_classic");
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
        if (e.error_code() == CardException::ErrorType::UNKOWN_ERROR)
        {
            // The reader returns the generic 0x6F00 signal a generic error
            // when the authentication failed.
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
