#include "pcsccardprobe.hpp"
#include "../../pcscreaderunit.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/logs.hpp"
#include "mifarecommands.hpp"
#include "mifareprofile.hpp"
#include <assert.h>
#include <desfirecommands.hpp>
#include <mifareultralightccommands.hpp>

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

        logicalaccess::MifareAccessInfo::SectorAccessBits sab;
        std::dynamic_pointer_cast<logicalaccess::MifareProfile>(chip->getProfile())
            ->setDefaultKeysAt(0x00);
        std::dynamic_pointer_cast<logicalaccess::MifareProfile>(chip->getProfile())
            ->setDefaultKeysAt(1);
        auto ret = command->readSector(1, 0, sab);
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
        DESFireCommands::DESFireCardVersion cardversion;
        auto desfire_command =
            std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
        desfire_command->selectApplication(0x00);
        desfire_command->getVersion(cardversion);

        if (uid)
            *uid =
                ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
        return true;
    }
    catch (const std::exception &)
    {
        // If an error occurred, the card probably isn't desfire.
        return false;
    }
}

bool PCSCCardProbe::is_desfire_ev1(std::vector<uint8_t> *uid)
{
    try
    {
        LLA_LOG_CTX("Probe::is_desfire_ev1");
        reset();
        auto chip = reader_unit_->createChip("DESFireEV1");
        DESFireCommands::DESFireCardVersion cardversion;
        auto desfire_command =
            std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
        assert(desfire_command);
        desfire_command->selectApplication(0x00);
        desfire_command->getVersion(cardversion);

        if (uid)
            *uid =
                ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
        return cardversion.softwareMjVersion >= 1;
    }
    catch (const std::exception &e)
    {
        // If an error occurred, the card probably isn't desfire.
        return false;
    }
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
	catch (const std::exception& e)
	{
		// TODO: handle the case authentication is not default by checking error code
		return false;
	}

	return true;
}


void PCSCCardProbe::reset()
{
    try
    {
        auto pcsc_ru = dynamic_cast<PCSCReaderUnit *>(reader_unit_);
        assert(pcsc_ru);

        pcsc_ru->reconnect(SCARD_UNPOWER_CARD);
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
        DESFireCommands::DESFireCardVersion cardversion;
        auto chip = reader_unit_->createChip("DESFireEV1");
        auto desfire_command =
            std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());
        assert(desfire_command);

        desfire_command->selectApplication(0x00);
        desfire_command->getVersion(cardversion);

        if (BufferHelper::allZeroes(cardversion.uid))
        {
            return true;
        }
        if (uid)
            *uid =
                ByteVector(std::begin(cardversion.uid), std::end(cardversion.uid));
        return false;
    }
    catch (const std::exception &)
    {
        return false;
    }
}