#include <logicalaccess/plugins/readers/pcsc/readers/cardprobes/pcsccardprobe.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderunit.hpp>

#include <logicalaccess/cards/chip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/mifare/mifarecommands.hpp>
#include <logicalaccess/plugins/cards/mifareultralight/mifareultralightccommands.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>

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
        auto command = std::dynamic_pointer_cast<MifareCommands>(chip->getCommands());

        EXCEPTION_ASSERT_WITH_LOG(command != nullptr,
            LibLogicalAccessException, "Failed to create MIFARE Classic commands while probing the card.");

        MifareAccessInfo::SectorAccessBits sector_access_bits;
        command->readSector(1, 0, std::shared_ptr<MifareKey>(), std::shared_ptr<MifareKey>(), sector_access_bits);
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
    catch (const std::exception &e)
    {
        // If an error occurred, the card probably isn't mifare classic.
        LOG(LogLevel::INFOS) << "MIFARE Classic probe failed : " << e.what();
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
        auto chip = reader_unit_->createChip(CMD_DESFIRE);
        auto desfire_command = std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());

        EXCEPTION_ASSERT_WITH_LOG(desfire_command != nullptr,
            LibLogicalAccessException, "Failed to create DESFire commands while probing the card.");

        desfire_command->selectApplication(0x00);
        const auto card_version = desfire_command->getVersion();

        if (uid)
            *uid = card_version.manufacturing.uid;
        return true;
    }
    catch (const std::exception &e)
    {
        // If an error occurred, the card probably isn't desfire.
        LOG(LogLevel::INFOS) << "DESFire probe failed : " << e.what();
        return false;
    }
}

PCSCCardProbe::DESFireVersionInfo PCSCCardProbe::get_desfire_version()
{
    // Keep DESFire command-layer details out of the public probe interface
    return probe_desfire_version();
}

PCSCCardProbe::DESFireVersionInfo PCSCCardProbe::probe_desfire_version()
{
    constexpr int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        try
        {
            LLA_LOG_CTX("Probe::get_desfire_version");
            reset();
            const auto chip = reader_unit_->createChip("DESFireEV1");
            const auto desfire_command = std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());

            EXCEPTION_ASSERT_WITH_LOG(desfire_command != nullptr,
                LibLogicalAccessException, "Failed to create DESFire commands while retrieving the card version.");

            desfire_command->selectApplication(0x00);
            const auto cardVersion = desfire_command->getVersion();

            return DESFireVersionInfo {
                cardVersion.hardware.majorVersion,
                cardVersion.software.majorVersion,
                cardVersion.manufacturing.uid
            };
        }
        catch (const CardException &e)
        {
            if (e.error_code() == CardException::FUNCTION_NOT_SUPPORTED)
            {
                // This likely means EV2 with Mandatory Proximity Check.
                // Note that unfortunately,
                // this will likely cause some issue with EV3 or something later.
                DESFireVersionInfo version;
                version.softwareMajorVersion = 2;
                return version;
            }

            throw;
        }
        catch (const std::exception &e)
        {
            LOG(LogLevel::INFOS) << "DESFire version probe attempt " << (attempt + 1)
                                 << "/" << maxAttempts << " failed : " << e.what();

            // Quite often the hardware is not ready or something else happens
            // which cause the command to fail. So we try a few time.
            // This seems to fix somewhat reliably the Desfire / EV1 / EV2 detection
            // issues.
            if (attempt + 1 == maxAttempts)
            {
                // If an error occurred, the card probably isn't desfire.
                LOG(LogLevel::INFOS) << "DESFire version probe failed after " << maxAttempts << " attempts.";
                return {};
            }
        }
    }

    // Defensive fallback in case the retry loop is changed in the future
    return {};
}

bool PCSCCardProbe::is_desfire_ev1(std::vector<uint8_t> *uid)
{
    LLA_LOG_CTX("Probe::is_desfire_ev1");
    const auto version = get_desfire_version();

    if (version.softwareMajorVersion != 1)
        return false;

    if (uid != nullptr)
        *uid = version.uid;

    return true;
}

bool PCSCCardProbe::is_desfire_ev2(std::vector<uint8_t> *uid)
{
    LLA_LOG_CTX("Probe::is_desfire_ev2");
    const auto version = get_desfire_version();

    if (version.softwareMajorVersion != 2)
        return false;

    if (uid != nullptr)
        *uid = version.uid;

    return true;
}

bool PCSCCardProbe::is_desfire_ev3(std::vector<uint8_t> *uid)
{
    LLA_LOG_CTX("Probe::is_desfire_ev3");
    const auto version = get_desfire_version();

    if (version.softwareMajorVersion != 3)
        return false;

    if (uid != nullptr)
        *uid = version.uid;

    return true;
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

        EXCEPTION_ASSERT_WITH_LOG(mfu_command != nullptr,
            LibLogicalAccessException, "Failed to create MIFARE Ultralight C commands while probing the card.");

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
  auto pcsc_ru = dynamic_cast<PCSCReaderUnit *>(reader_unit_);

  EXCEPTION_ASSERT_WITH_LOG(pcsc_ru != nullptr,
      LibLogicalAccessException, "Card probing failed because the reader unit is not a PCSCReaderUnit.");

  try
  {
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
    // Note that if we are already authenticated against the card
    // this will not work. This should never be a problem because
    // PCSCCardProbe is only used at card detection.
    try
    {
        auto pcsc_ru = dynamic_cast<PCSCReaderUnit *>(reader_unit_);
        EXCEPTION_ASSERT_WITH_LOG(pcsc_ru != nullptr,
            LibLogicalAccessException, "No PCSC reader unit available for DESFire random UID detection.");
        auto chip = reader_unit_->createChip("DESFireEV1");
        auto desfire_command = std::dynamic_pointer_cast<DESFireCommands>(chip->getCommands());

        EXCEPTION_ASSERT_WITH_LOG(desfire_command != nullptr,
            LibLogicalAccessException, "Failed to create DESFire commands while checking for a random UID.");

		if (!pcsc_ru->getPCSCConfiguration()->getSkipCSN())
		{
			auto csn = pcsc_ru->getCardSerialNumber();
			// 0x08 as first byte means random UID.
			if (csn.at(0) == 0x08 || csn.at(0) == 0x80 /* DESFire EV1 legacy random UID */) {
				return true;
			}
			if (uid)
				*uid = ByteVector(std::begin(csn), std::end(csn));
		}
        return false;
    }
    catch (const std::exception &)
    {
        return false;
    }
}
