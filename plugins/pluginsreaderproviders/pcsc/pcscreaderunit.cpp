/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include "pcscreaderunit.hpp"

#include <iomanip>
#include <thread>

#include "pcscreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"

#include "commands/samav1iso7816commands.hpp"
#include "commands/samav2iso7816commands.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "commands/mifarepcsccommands.hpp"
#include "commands/mifarescmcommands.hpp"
#include "commands/mifare_acr1222L_commands.hpp"
#include "commands/mifarecherrycommands.hpp"
#include "commands/mifarespringcardcommands.hpp"
#include "commands/iso15693pcsccommands.hpp"
#include "commands/twiciso7816commands.hpp"
#include "commands/mifareultralightpcsccommands.hpp"
#include "commands/mifareultralightcpcsccommands.hpp"
#include "commands/mifareultralightcomnikeyxx21commands.hpp"
#include "commands/mifareultralightcomnikeyxx22commands.hpp"
#include "commands/mifareultralightcacsacrcommands.hpp"
#include "commands/mifareultralightcspringcardcommands.hpp"
#include "commands/mifareomnikeyxx21commands.hpp"
#include "commands/mifareplus_omnikeyxx21_sl1.hpp"
#include "commands/mifareplus_sprincard_sl1.hpp"
#include "commands/topazpcsccommands.hpp"
#include "commands/topazacsacrcommands.hpp"
#include "commands/topazscmcommands.hpp"
#include "commands/topazomnikeyxx27commands.hpp"
#include "commands/proxcommand.hpp"
#include "commands/felicascmcommands.hpp"
#include "commands/felicaspringcardcommands.hpp"
#include "MifarePlusSL0Commands.hpp"
#include "epasscommands.hpp"
#include "epassreadercardadapter.hpp"

#include "samav1chip.hpp"
#include "samav2chip.hpp"
#include "mifarepluschip.hpp"
#include "MifarePlusSL1Chip.hpp"
#include "desfireev1chip.hpp"
#include "mifare1kchip.hpp"
#include "mifare4kchip.hpp"
#include "iso7816chip.hpp"
#include "twicchip.hpp"
#include "iso15693chip.hpp"
#include "tagitchip.hpp"
#include "mifareultralightcchip.hpp"
#include "proxchip.hpp"
#include "felicachip.hpp"
#include "epasschip.hpp"
#include "topazchip.hpp"
#include "generictagchip.hpp"

#include "commands/samiso7816resultchecker.hpp"
#include "commands/desfireiso7816resultchecker.hpp"
#include "commands/mifareplusiso7816resultchecker.hpp"
#include "commands/mifareomnikeyxx27resultchecker.hpp"
#include "commands/acsacrresultchecker.hpp"
#include "commands/springcardresultchecker.hpp"

#include "readers/omnikeyxx21readerunit.hpp"
#include "readers/omnikeylanxx21readerunit.hpp"
#include "readers/omnikeyxx25readerunit.hpp"
#include "readers/cherryreaderunit.hpp"
#include "readers/springcardreaderunit.hpp"
#include "readers/acsacrreaderunit.hpp"

#include "pcscdatatransport.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "logicalaccess/cardprobe.hpp"
#include <logicalaccess/utils.hpp>

#include "readers/acsacr1222llcddisplay.hpp"


#include "logicalaccess/settings.hpp"
#include "pcsc_ctl_datatransport.hpp"
#include "commands/mifareplus_acsacr1222l_sl1.hpp"
#include "commands/mifareplus_pcsc_sl3.hpp"
#include "commands/mifare_cl1356_commands.hpp"
#include "readers/cardprobes/pcsccardprobe.hpp"
#include "atrparser.hpp"
#include "commands/id3resultchecker.hpp"

#include <cstring>

namespace logicalaccess
{
    PCSCReaderUnit::PCSCReaderUnit(const std::string& name)
        : ISO7816ReaderUnit(READER_PCSC), d_name(name), d_connectedName(name)
    {
		d_card_type = CHIP_UNKNOWN;

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/PCSCReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", CHIP_UNKNOWN);
        }
        catch (...) {}

        d_proxyReaderUnit.reset();
        d_readerUnitConfig.reset(new PCSCReaderUnitConfiguration());
        setDefaultReaderCardAdapter(std::shared_ptr<PCSCReaderCardAdapter>(new PCSCReaderCardAdapter()));
    }

    PCSCReaderUnit::~PCSCReaderUnit()
    {
        // We want to destroy the Chip object before destroying the reader unit.
        // We have to access the card's type before nulling the card object.
        std::string genericCardType;
        if (d_proxyReaderUnit && d_proxyReaderUnit->d_insertedChip)
            genericCardType = d_proxyReaderUnit->d_insertedChip->getGenericCardType();
        else if (d_insertedChip)
            genericCardType = d_insertedChip->getGenericCardType();

        d_insertedChip = nullptr;
        if (d_proxyReaderUnit)
            d_proxyReaderUnit->d_insertedChip = nullptr;

        LOG(LogLevel::INFOS) << "Reader unit destruction...";

        if (isConnected())
        {
            disconnect();
        }
    }

    std::string PCSCReaderUnit::getName() const
    {
        return d_name;
    }

    void PCSCReaderUnit::setName(const std::string& name)
    {
        d_name = name;

        if (this->getPCSCType() == PCSC_RUT_DEFAULT)
        {
            d_proxyReaderUnit = PCSCReaderUnit::createPCSCReaderUnit(d_name);
            d_proxyReaderUnit->makeProxy(std::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()),
                                         getPCSCConfiguration());
        }
    }

    std::string PCSCReaderUnit::getConnectedName()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getName();
        }

        return ((d_connectedName != "") ? d_connectedName : d_name);
    }

    const SCARDHANDLE& PCSCReaderUnit::getHandle() const
    {
        static SCARDHANDLE nullhandle = 0;
        if (connection_)
            return connection_->handle_;
        return nullhandle;
    }

    void PCSCReaderUnit::serialize(boost::property_tree::ptree& parentNode)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->serialize(parentNode);
        }
        else
        {
            boost::property_tree::ptree node;
            ReaderUnit::serialize(node);
            node.put("Name", d_name);
            parentNode.add_child(getDefaultXmlNodeName(), node);
        }
    }

    void PCSCReaderUnit::unSerialize(boost::property_tree::ptree& node)
    {
        d_name = node.get_child("Name").get_value<std::string>();
        std::shared_ptr<PCSCReaderUnitConfiguration> pcscRUC = getPCSCConfiguration();
        PCSCReaderUnitType pcscType = getPCSCType();
        d_proxyReaderUnit = PCSCReaderUnit::createPCSCReaderUnit(d_name);
        if (d_proxyReaderUnit->getPCSCType() == pcscType)
        {
            d_proxyReaderUnit.reset();
        }
        else
        {
            d_proxyReaderUnit->makeProxy(std::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);
        }

        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->unSerialize(node);
        }
        else
        {
            ReaderUnit::unSerialize(node);
        }
    }

    std::shared_ptr<PCSCReaderUnit> PCSCReaderUnit::createPCSCReaderUnit(const std::string& readerName)
    {
        std::shared_ptr<ReaderUnit> reader = LibraryManager::getInstance()->getReader(readerName);
        if (reader)
            assert(std::dynamic_pointer_cast<PCSCReaderUnit>(reader));
        else
        {
            reader = std::make_shared<PCSCReaderUnit>(readerName);
        }
        return std::dynamic_pointer_cast<PCSCReaderUnit>(reader);
    }

    PCSCReaderUnitType PCSCReaderUnit::getPCSCType() const
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getPCSCType();
        }

        return PCSC_RUT_DEFAULT;
    }

    bool PCSCReaderUnit::isConnected()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->isConnected();
        }
        return connection_ && connection_->handle_;
    }

    bool PCSCReaderUnit::waitInsertion(unsigned int maxwait)
    {
        if (isConnected())
        {
            LOG(LogLevel::ERRORS) << EXCEPTION_MSG_CONNECTED;
            disconnect();
        }

        if (Settings::getInstance()->SeeWaitInsertionLog)
        {
            LOG(LogLevel::INFOS) << "Waiting card insertion...";
        }
        teardown_pcsc_connection();


        SPtrStringVector readers_names;
        ReaderStateVector readers;
        std::tie(readers_names, readers) = prepare_poll_parameters();
        ElapsedTimeCounter time_counter;
        do
        {
            LONG r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(),
                                     ((maxwait == 0) ? INFINITE : maxwait - time_counter.elapsed()),
                                     &readers[0], readers.size());
            if (SCARD_S_SUCCESS == r)
            {
                for (size_t i = 0; i < readers.size(); ++i)
                {
                    if ((SCARD_STATE_CHANGED & readers[i].dwEventState) != 0)
                    {
                        readers[i].dwCurrentState = readers[i].dwEventState;
                        if ((SCARD_STATE_PRESENT & readers[i].dwEventState) != 0)
                        {
                            // The current reader detected a card. Great, let's use it.

                            std::string connectedReader;
                            std::string cardType;
                            atr_ = std::vector<uint8_t>(readers[i].rgbAtr,
                                                        readers[i].rgbAtr + readers[i].cbAtr);

                            // Create the proxy now, so the ATR parser operate on the correct
                            // reader type. -- This help with some reader-specific ATR.
                            connectedReader = std::string(readers[i].szReader);
                            waitInsertion_create_proxy(connectedReader);
                            cardType = ATRParser::guessCardType(atr_, getPCSCType());
                            LOG(INFOS) << "Guessed card type from atr: " << cardType;
                            return process_insertion(cardType, maxwait, time_counter);
                        }
                    }
                }
            }
            else if (r != SCARD_E_TIMEOUT)
            {
                if (Settings::getInstance()->SeeWaitInsertionLog)
                {
                    LOG(LogLevel::ERRORS) << "Cannot get status change: " << r << ".";
                }
                PCSCDataTransport::CheckCardError(static_cast<unsigned int>(r));
                break;
            }
        } while (maxwait == 0 || time_counter.elapsed() < maxwait);
        return false;
    }

    bool PCSCReaderUnit::waitRemoval(unsigned int maxwait)
    {
        if (d_proxyReaderUnit)
        {
            if (d_proxyReaderUnit->waitRemoval(maxwait))
            {
                if (d_name == "")
                {
                    d_insertedChip = nullptr;
                    d_proxyReaderUnit.reset();
                }
                return true;
            }

            return false;
        }

		disconnect();
		assert(connection_ == nullptr);
        if (isConnected())
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CONNECTED);
        }

        if (Settings::getInstance()->SeeWaitRemovalLog)
        {
            LOG(LogLevel::INFOS) << "Waiting card removal...";
        }

        std::string reader = getConnectedName();

        int readers_count = static_cast<int>(getReaderProvider()->getReaderList().size());

        if (reader != "")
        {
            readers_count = 1;
        }

        if (readers_count == 0)
        {
            THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_NOREADER);
        }

        char** readers_names = new char*[readers_count];

        for (int i = 0; i < readers_count; ++i)
        {
            readers_names[i] = NULL;
        }

        SCARD_READERSTATE* readers = new SCARD_READERSTATE[readers_count];

        memset(readers, 0x00, sizeof(SCARD_READERSTATE) * readers_count);

        if (reader != "")
        {
            readers_names[0] = strdup(reader.c_str());
            readers[0].dwCurrentState = SCARD_STATE_UNAWARE;
            readers[0].dwEventState = SCARD_STATE_UNAWARE;
            readers[0].szReader = reinterpret_cast<const char*>(readers_names[0]);
        }
        else
        {
            for (int i = 0; i < readers_count; ++i)
            {
                readers_names[i] = strdup(getReaderProvider()->getReaderList().at(i)->getName().c_str());
                readers[i].dwCurrentState = SCARD_STATE_UNAWARE;
                readers[i].dwEventState = SCARD_STATE_UNAWARE;
                readers[i].szReader = reinterpret_cast<const char*>(readers_names[i]);
            }
        }

        reader.clear();

        LONG r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), ((maxwait == 0) ? INFINITE : maxwait), readers, readers_count);

        if (SCARD_S_SUCCESS == r)
        {
            for (int i = 0; i < readers_count; ++i)
            {
                if ((SCARD_STATE_PRESENT & readers[i].dwEventState) == 0)
                {
                    reader = std::string(reinterpret_cast<const char*>(readers[i].szReader), strlen(reinterpret_cast<const char*>(readers[i].szReader)));
                    break;
                }
            }

            if (reader == "")
            {
                for (int i = 0; i < readers_count; ++i)
                {
                    readers[i].dwCurrentState = readers[i].dwEventState;
                }

                bool loop;
                ElapsedTimeCounter time_counter;
                do
                {
                    loop = false;
                    r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), ((maxwait == 0) ? INFINITE : maxwait), readers, readers_count);

                    if (SCARD_S_SUCCESS == r)
                    {
                        if ((maxwait == 0) || time_counter.elapsed() < maxwait)
                        {
                            loop = true;
                        }
                        for (int i = 0; i < readers_count; ++i)
                        {
                            if ((SCARD_STATE_PRESENT & readers[i].dwEventState) == 0)
                            {
                                loop = false;

                                reader = std::string(reinterpret_cast<const char*>(readers[i].szReader), strlen(reinterpret_cast<const char*>(readers[i].szReader)));
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (r != SCARD_E_TIMEOUT)
                        {
                            if (Settings::getInstance()->SeeWaitRemovalLog)
                            {
                                LOG(LogLevel::ERRORS) << "Cannot get status change: " << r << ".";
                            }
                            PCSCDataTransport::CheckCardError(r);
                        }
                    }
                } while (loop);
            }
        }

        for (int i = 0; i < readers_count; ++i)
        {
            if (readers_names[i])
            {
                free(readers_names[i]);
                readers_names[i] = NULL;
            }
        }

        delete[] readers;
        readers = NULL;

        delete[] readers_names;
        readers_names = NULL;

        if (!reader.empty())
        {
            if (d_name == "")
            {
                d_proxyReaderUnit.reset();
            }
            d_insertedChip.reset();
            d_connectedName = d_name;
        }
		
		assert(connection_ == nullptr);
        return (!reader.empty());
    }

    bool PCSCReaderUnit::connect()
    {
        return connect(getPCSCConfiguration()->getShareMode());
    }

    bool PCSCReaderUnit::connect(PCSCShareMode share_mode)
    {
        LOG(LogLevel::INFOS) << "Connecting to the chip... Share mode {" << share_mode << "}";
        bool ret = false;
        if (d_proxyReaderUnit)
        {
            LOG(LogLevel::INFOS) << "Need to use a proxy reader !";
            ret = d_proxyReaderUnit->connect(share_mode);
        }
        else
        {
            if (isConnected())
            {
                disconnect();
            }
			setup_pcsc_connection(share_mode);

			if (d_insertedChip->getGenericCardType() == CHIP_SAM)
				connection_->setDisposition(SCARD_UNPOWER_CARD);
			else
				connection_->setDisposition(SCARD_LEAVE_CARD);

                LOG(LogLevel::INFOS) << "SCardConnect Success !";
                detect_mifareplus_security_level(d_insertedChip);

            d_insertedChip = adjustChip(d_insertedChip);
            if (d_proxyReaderUnit)
            {
                d_proxyReaderUnit->setSingleChip(d_insertedChip);
            }
            ret = true;
        }

        if (ret)
        {
            // Unknown T=CL card, we have to send specific reader command to determine the type (A or B ?)
            if (d_insertedChip && d_insertedChip->getCardType() == "GENERIC_T_CL")
            {
                LOG(LogLevel::INFOS) << "Inserted chip is {CT_GENERIC_T_CL}. Looking for type A or B...";
                if (d_proxyReaderUnit)
                {
                    setSingleChip(d_proxyReaderUnit->getSingleChip());
                }
                else
                {
                    bool isTypeA = false;
                    bool isTypeB = false;
                    getT_CL_ISOType(isTypeA, isTypeB);

                    std::shared_ptr<Chip> newChip;
                    if (isTypeA)
                    {
                        LOG(LogLevel::INFOS) << "Type A detected !";
                        newChip = createChip("GENERIC_T_CL_A");
                    }
                    else if (isTypeB)
                    {
                        LOG(LogLevel::INFOS) << "Type B detected !";
                        newChip = createChip("GENERIC_T_CL_B");
                    }

                    if (newChip)
                    {
                        newChip->setChipIdentifier(d_insertedChip->getChipIdentifier());
                        d_insertedChip = newChip;
                    }
                    else
                    {
                        LOG(LogLevel::INFOS) << "Unable to find the type !";
                    }
                }
            }
        }
        if (ret)
            cardConnected();
        return ret;
    }

    bool PCSCReaderUnit::reconnect(int action)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->reconnect();
        }

		connection_->reconnect();

		if (!ISO7816ReaderUnit::reconnect(action))
			return false;

        if (!isConnected())
        {
            return false;
        }
        return false;
    }

    void PCSCReaderUnit::disconnect()
    {
		LOG(LogLevel::INFOS) << "Disconnecting from the chip.";

		if (d_proxyReaderUnit)
		{
			d_proxyReaderUnit->disconnect();
		}
		else
		{
			if (isConnected())
			{
				teardown_pcsc_connection();
			}
		}
    }

    bool PCSCReaderUnit::connectToReader()
    {
        LOG(LogLevel::INFOS) << "Connecting to reader...";

        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->connectToReader();
        }

        return ISO7816ReaderUnit::connectToReader();
    }

    void PCSCReaderUnit::disconnectFromReader()
    {
        LOG(LogLevel::INFOS) << "Disconnecting from reader...";

        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->disconnectFromReader();
        }
        else
        {
            ISO7816ReaderUnit::disconnectFromReader();
            teardown_pcsc_connection();
        }
    }

    std::vector<unsigned char> PCSCReaderUnit::getCardSerialNumber()
    {
        std::vector<unsigned char> ucReceivedData;

        ucReceivedData = getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xCA, 0x00, 0x00, 0x00);

        return std::vector<unsigned char>(ucReceivedData.begin(), ucReceivedData.end() - 2);
    }

	const std::vector<uint8_t>& PCSCReaderUnit::getATR() const
    {
        return atr_;
    }

     std::shared_ptr<ReaderCardAdapter> PCSCReaderUnit::getReaderCardAdapter(std::string type)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getReaderCardAdapter(type);
        }

        return getDefaultReaderCardAdapter();
    }

    std::shared_ptr<Chip> PCSCReaderUnit::createChip(std::string type)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->createChip(type);
        }

        std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
        if (chip)
        {
            LOG(LogLevel::INFOS) << "Chip (" << chip->getCardType() <<
            ") created, creating other associated objects... Reader PCSC Type is " << getPCSCType();
            type = chip->getCardType(); // type may not be what we expected, it may be unsupported.

            std::shared_ptr<ReaderCardAdapter> rca = getReaderCardAdapter(type);
            std::shared_ptr<Commands> commands;
            std::shared_ptr<ResultChecker> resultChecker= createDefaultResultChecker();

			if (type == CHIP_MIFARE1K || type == CHIP_MIFARE4K || type == CHIP_MIFARE)
            {
				if (getPCSCType() == PCSC_RUT_SCM)
                {
                    commands.reset(new MifareSCMCommands());
                }
                else if (getPCSCType() == PCSC_RUT_CHERRY)
                {
                    commands.reset(new MifareCherryCommands());
                }
                else if (getPCSCType() == PCSC_RUT_SPRINGCARD)
                {
                    commands.reset(new MifareSpringCardCommands());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX27)
                {
                    resultChecker.reset(new MifareOmnikeyXX27ResultChecker());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_LAN_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_XX22)
                {
                    commands.reset(new MifareOmnikeyXX21Commands());
                }
                else if (getPCSCType() == PCSC_RUT_ACS_ACR_1222L)
                {
                    commands.reset(new MifareACR1222LCommands());
                }
                else if (getPCSCType() == PCSC_RUT_ID3_CL1356)
                {
                    commands.reset(new MifareCL1356Commands());
                    resultChecker = std::make_shared<ID3ResultChecker>();
                }
                else
                {
                    commands.reset(new MifarePCSCCommands());
                }
            }
            else if (chip->getGenericCardType() == "HIDiClass")
            {
                // HID iClass cards have a lot of restriction on license use from HID Global, so we try to load it dynamically if the dynamic library is side by side, otherwise we don't mind.
                commands = chip->getCommands();
                if (commands)
                {
                    rca = commands->getReaderCardAdapter();
                }
                else
                {
                    LOG(LogLevel::WARNINGS) << "Cannot found HIDiClass commands.";
                }
            }
            else if (type == CHIP_DESFIRE_EV1)
            {
                commands.reset(new DESFireEV1ISO7816Commands());
                std::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
                resultChecker.reset(new DESFireISO7816ResultChecker());
            }
            else if (type == CHIP_DESFIRE)
            {
                commands.reset(new DESFireISO7816Commands());
                std::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
                resultChecker.reset(new DESFireISO7816ResultChecker());
            }
            else if (type == CHIP_ISO15693)
            {
                commands.reset(new ISO15693PCSCCommands());
            }
            else if (type == CHIP_ISO7816)
            {
                commands.reset(new ISO7816ISO7816Commands());
            }
            else if (type == CHIP_TAGIT)
            {
                commands.reset(new ISO15693PCSCCommands());
            }
            else if (type == CHIP_TWIC)
            {
                commands.reset(new TwicISO7816Commands());
            }
			else if (type == CHIP_MIFAREULTRALIGHT)
            {
                commands.reset(new MifareUltralightPCSCCommands());
            }
			else if (type == CHIP_MIFAREULTRALIGHTC)
            {
                if (getPCSCType() == PCSC_RUT_ACS_ACR || getPCSCType() == PCSC_RUT_ACS_ACR_1222L)
                {
                    commands.reset(new MifareUltralightCACSACRCommands());
                }
                else if (getPCSCType() == PCSC_RUT_SPRINGCARD)
                {
                    commands.reset(new MifareUltralightCSpringCardCommands());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX21)
                {
                    commands.reset(new MifareUltralightCOmnikeyXX21Commands());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX22)
                {
                    commands.reset(new MifareUltralightCOmnikeyXX22Commands());
                }
                else
                {
                    commands.reset(new MifareUltralightCPCSCCommands());
                }
            }
			else if (type == CHIP_SAMAV1)
            {
                commands.reset(new SAMAV1ISO7816Commands());
                std::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
                std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(commands)->setCrypto(samcrypto);
                resultChecker.reset(new SAMISO7816ResultChecker());
            }
            else if (type == CHIP_SAMAV2)
            {
                commands.reset(new SAMAV2ISO7816Commands());
                std::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
                std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(commands)->setCrypto(samcrypto);
                resultChecker.reset(new SAMISO7816ResultChecker());
            }
            else if (type.find("MifarePlus") == 0)
            {
                configure_mifareplus_chip(chip, commands, resultChecker);
            }
            else if (type == CHIP_PROX)
            {   // A dummy command, whose only goal is to allow retrieval of the
                // reader unit later on.
                commands.reset(new ProxCommand());
            }
			else if (type == CHIP_FELICA)
			{
				if (getPCSCType() == PCSC_RUT_SCM)
				{
					commands.reset(new FeliCaSCMCommands());
				}
                else if (getPCSCType() == PCSC_RUT_SPRINGCARD)
                {
                    commands.reset(new FeliCaSpringCardCommands());
                }
			}
            else if (type == CHIP_EPASS)
            {
                commands = std::make_shared<EPassCommand>();
                rca = std::make_shared<EPassReaderCardAdapter>();
                rca->setDataTransport(std::make_shared<PCSCDataTransport>());
                //commands->setReaderCardAdapter(rca);
            }
            else if (type == CHIP_TOPAZ)
            {
                if (getPCSCType() == PCSC_RUT_ACS_ACR || getPCSCType() == PCSC_RUT_ACS_ACR_1222L)
                {
                    commands.reset(new TopazACSACRCommands());
                }
                else if (getPCSCType() == PCSC_RUT_SCM)
                {
                    commands.reset(new TopazSCMCommands());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX27)
                {
                    commands.reset(new TopazOmnikeyXX27Commands());
                }
                else
                {
                    commands.reset(new TopazPCSCCommands());
                }
            }

			if (type == CHIP_DESFIRE || type == CHIP_DESFIRE_EV1)
            {
                std::shared_ptr<DESFireISO7816Commands> dcmd = std::dynamic_pointer_cast<DESFireISO7816Commands>(commands);
                if (dcmd->getSAMChip())
                {
                    std::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
					if (dcmd->getSAMChip()->getCardType() == CHIP_SAMAV1)
                        std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(dcmd->getSAMChip()->getCommands())->setCrypto(samcrypto);
					else if (dcmd->getSAMChip()->getCardType() == CHIP_SAMAV2)
                        std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(dcmd->getSAMChip()->getCommands())->setCrypto(samcrypto);
                }
            }

            LOG(LogLevel::INFOS) << "Other objects created, making association (ReaderCardAdapter empty? " << !rca << " - Commands empty? " << !commands << ")...";

            if (rca)
            {
				rca->setResultChecker(resultChecker);
                std::shared_ptr<DataTransport> dt = getDataTransport();
                if (dt)
                {
                    LOG(LogLevel::INFOS) << "Data transport forced to a specific one.";
                    rca->setDataTransport(dt);
                }

                dt = rca->getDataTransport();
                if (dt)
                {
                    dt->setReaderUnit(shared_from_this());
                }
            }

            if (commands)
            {
                commands->setReaderCardAdapter(rca);
                commands->setChip(chip);
                chip->setCommands(commands);
            }

            LOG(LogLevel::INFOS) << "Object creation done.";
        }
        return chip;
    }

    std::shared_ptr<Chip> PCSCReaderUnit::getSingleChip()
    {
        std::shared_ptr<Chip> chip;
        if (d_proxyReaderUnit)
            chip = d_proxyReaderUnit->getSingleChip();
        else
            chip = d_insertedChip;
        return chip;
    }

    std::vector<std::shared_ptr<Chip> > PCSCReaderUnit::getChipList()
    {
        if (d_proxyReaderUnit)
            return d_proxyReaderUnit->getChipList();
        std::vector<std::shared_ptr<Chip> > chipList;
        std::shared_ptr<Chip> singleChip = getSingleChip();
        if (singleChip)
        {
            chipList.push_back(singleChip);
        }
        return chipList;
    }

    std::shared_ptr<PCSCReaderCardAdapter> PCSCReaderUnit::getDefaultPCSCReaderCardAdapter()
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getDefaultReaderCardAdapter());
    }

    std::string PCSCReaderUnit::getReaderSerialNumber()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getReaderSerialNumber();
        }
        return "";
    }

    std::shared_ptr<PCSCReaderProvider> PCSCReaderUnit::getPCSCReaderProvider() const
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getPCSCReaderProvider();
        }
        return std::dynamic_pointer_cast<PCSCReaderProvider>(getReaderProvider());
    }

    void PCSCReaderUnit::makeProxy(std::shared_ptr<PCSCReaderUnit> readerUnit, std::shared_ptr<PCSCReaderUnitConfiguration> readerUnitConfig)
    {
        d_card_type = readerUnit->getCardType();
        d_readerProvider = readerUnit->getReaderProvider();
        if (getPCSCConfiguration()->getPCSCType() == readerUnitConfig->getPCSCType())
        {
            d_readerUnitConfig = readerUnitConfig;
        }

		atr_ = readerUnit->getATR();
        d_insertedChip = readerUnit->getSingleChip();
    }

    void PCSCReaderUnit::setSingleChip(std::shared_ptr<Chip> chip)
    {
        d_insertedChip = chip;
    }

    std::shared_ptr<ReaderUnitConfiguration> PCSCReaderUnit::getConfiguration()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getConfiguration();
        }

        return d_readerUnitConfig;
    }

    void PCSCReaderUnit::setConfiguration(std::shared_ptr<ReaderUnitConfiguration> config)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setConfiguration(config);
        }
        else
        {
            d_readerUnitConfig = config;
        }
	}

    void PCSCReaderUnit::changeReaderKey(std::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->changeReaderKey(keystorage, key);
        }
    }

    void PCSCReaderUnit::getT_CL_ISOType(bool& isTypeA, bool& isTypeB)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->getT_CL_ISOType(isTypeA, isTypeB);
        }
        else
        {
            isTypeA = false;
            isTypeB = false;
        }
    }

    std::shared_ptr<PCSCReaderUnit> PCSCReaderUnit::getProxyReaderUnit()
    {
        return d_proxyReaderUnit;
    }

    TechnoBitset PCSCReaderUnit::getCardTechnologies()
    {
        if (d_proxyReaderUnit)
            return d_proxyReaderUnit->getCardTechnologies();
        return ReaderUnit::getCardTechnologies();
    }

    void PCSCReaderUnit::setCardTechnologies(const TechnoBitset &bitset)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setCardTechnologies(bitset);
            return;
        }
        ReaderUnit::setCardTechnologies(bitset);
    }

    std::shared_ptr<LCDDisplay> PCSCReaderUnit::getLCDDisplay()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getLCDDisplay();
        }
        return ReaderUnit::getLCDDisplay();
    }

    void PCSCReaderUnit::setLCDDisplay(std::shared_ptr<LCDDisplay> d)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->setLCDDisplay(d);
        }
        ReaderUnit::setLCDDisplay(d);
    }

    std::shared_ptr<LEDBuzzerDisplay> PCSCReaderUnit::getLEDBuzzerDisplay()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getLEDBuzzerDisplay();
        }
        return ReaderUnit::getLEDBuzzerDisplay();
    }

    void
    PCSCReaderUnit::setLEDBuzzerDisplay(std::shared_ptr<LEDBuzzerDisplay> lbd)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->setLEDBuzzerDisplay(lbd);
        }
        ReaderUnit::setLEDBuzzerDisplay(lbd);
    }

    ReaderServicePtr PCSCReaderUnit::getService(const ReaderServiceType &type)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getService(type);
        }
        return ReaderUnit::getService(type);
    }

    unsigned long PCSCReaderUnit::getActiveProtocol() const
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getActiveProtocol();
        }
        if (connection_)
            return connection_->active_protocol_;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "No active connection.");
    }

    PCSCShareMode PCSCReaderUnit::getShareMode() const
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getShareMode();
        }
        if (connection_)
            return connection_->share_mode_;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "No active connection.");
    }

    std::shared_ptr<SAMChip> PCSCReaderUnit::getSAMChip()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getSAMChip();
        }

        return ISO7816ReaderUnit::getSAMChip();
    }

    void PCSCReaderUnit::setSAMChip(std::shared_ptr<SAMChip> t)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setSAMChip(t);
        }

        ISO7816ReaderUnit::setSAMChip(t);
    }

    std::shared_ptr<ISO7816ReaderUnit> PCSCReaderUnit::getSAMReaderUnit()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getSAMReaderUnit();
        }

        return ISO7816ReaderUnit::getSAMReaderUnit();
    }

    void PCSCReaderUnit::setSAMReaderUnit(std::shared_ptr<ISO7816ReaderUnit> t)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setSAMReaderUnit(t);
        }

        ISO7816ReaderUnit::setSAMReaderUnit(t);
    }

    void PCSCReaderUnit::setup_pcsc_connection(PCSCShareMode share_mode)
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->setup_pcsc_connection(share_mode);
        }
        assert(connection_ == nullptr);
        if (share_mode == SC_DIRECT)
        {
            assert(getPCSCReaderProvider());
            connection_ = std::unique_ptr<PCSCConnection>(new PCSCConnection(
                SC_DIRECT,
                0, // No protocol
                getPCSCReaderProvider()->getContext(), getConnectedName()));

            auto ctl_data_transport =
                std::make_shared<PCSCControlDataTransport>();
            ctl_data_transport->setReaderUnit(shared_from_this());
            getDefaultReaderCardAdapter()->setResultChecker(createDefaultResultChecker());
            getDefaultReaderCardAdapter()->setDataTransport(ctl_data_transport);
        }
        else
        {
            connection_ = std::unique_ptr<PCSCConnection>(new PCSCConnection(
                share_mode, getPCSCConfiguration()->getTransmissionProtocol(),
				getPCSCReaderProvider()->getContext(), getConnectedName()));

			auto data_transport = std::make_shared<PCSCDataTransport>();
			data_transport->setReaderUnit(shared_from_this());
			getDefaultReaderCardAdapter()->setDataTransport(data_transport);
        }
    }

    void PCSCReaderUnit::teardown_pcsc_connection()
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->teardown_pcsc_connection();
        }
        connection_ = nullptr;
    }

    void PCSCReaderUnit::configure_mifareplus_chip(std::shared_ptr<Chip> chip,
                                                   std::shared_ptr<Commands> &commands,
                                                   std::shared_ptr<ResultChecker> &resultChecker)
    {
        // It's possible we are in security level 1. In this case,
        // The chip type is MifarePlusSL1Chip (MifarePlusSL1_2kChip or 4k).
        // We just need to plug the command object for SL1 Mifare.
        auto mfp_sl1 = std::dynamic_pointer_cast<MifarePlusSL1Chip>(chip);
        if (mfp_sl1)
        {
            LOG(INFOS) << "The mifare plus is in security level 1.";

            if (getPCSCType() == PCSC_RUT_OMNIKEY_XX27)
            {
                auto cmd = new MifarePlusSL1Policy<MifarePlusSL1PCSCCommands,
                        MifarePCSCCommands>();
                commands.reset(cmd);
                resultChecker.reset(new MifareOmnikeyXX27ResultChecker());
            }
            else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_XX22 /* TODO: check it is really the same APDU for Omnikey 5022-CL */)
            {
                commands.reset(new MifarePlusSL1Policy<
                        MifarePlusOmnikeyXX21SL1Commands,
                        MifareOmnikeyXX21Commands>);
            }
            else if (getPCSCType() == PCSC_RUT_ACS_ACR_1222L)
            {
                commands.reset(new MifarePlusACSACR1222L_SL1Commands());
            }
            else if (getPCSCType() == PCSC_RUT_SPRINGCARD)
            {
                auto cmd = new MifarePlusSL1Policy<
                        MifarePlusSpringcardAES_SL1_Auth,
                        MifareSpringCardCommands>();
                commands.reset(cmd);
            }
            else
            {
                commands.reset(new MifarePlusSL1PCSCCommands());
            }

            return;
        }

        LOG(INFOS) << "Detected some kind of MifarePlus..." << chip->getCardType();
        auto mfp = std::dynamic_pointer_cast<MifarePlusChip>(chip);
        assert(mfp);
        if (mfp->getSecurityLevel() == 0)
        {
            LOG(INFOS) << "The mifare plus is in security level 0.";
            commands.reset(new MifarePlusSL0Commands());
        }
        else if (mfp->getSecurityLevel() == 3)
        {
            LOG(INFOS) << "The mifare plus is in security level 3.";
            commands.reset(new MifarePlusSL3PCSCCommands());
        }
        else if (mfp->getSecurityLevel() == -1) // unknown
        {
            commands.reset(new MifarePlusSL0Commands()); // will be used to detect the security level.
        }
        resultChecker = std::make_shared<MifarePlusISO7816ResultChecker>();
    }

    void PCSCReaderUnit::detect_mifareplus_security_level(std::shared_ptr<Chip> c)
    {
        if (c->getCardType() == "MifarePlusS" || c->getCardType() == "MifarePlusX")
        {
            auto mfp = std::dynamic_pointer_cast<MifarePlusChip>(c);
            assert(mfp);

            if (mfp->getSecurityLevel() != -1)
                return; // we already know the current SL.

            std::shared_ptr<MifarePlusSL0Commands> mfp_utils = std::dynamic_pointer_cast<MifarePlusSL0Commands>(c->getCommands());
            assert(mfp_utils);

            if (mfp_utils->probeLevel3())
            {
                d_insertedChip = createChip("MifarePlus_SL3_2K");
            }
            else if (mfp_utils->isLevel0()) // a bit dangerous as it overwrite data on card
            {
                if (mfp_utils->is4K())
                    d_insertedChip = createChip("MifarePlus_SL0_4K");
                else
                    d_insertedChip = createChip("MifarePlus_SL0_2K");
            }
        }
    }

    std::shared_ptr<ResultChecker> PCSCReaderUnit::createDefaultResultChecker() const
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->createDefaultResultChecker();
        }
        return ISO7816ReaderUnit::createDefaultResultChecker();
    }

    std::shared_ptr<CardProbe> PCSCReaderUnit::createCardProbe()
    {
        return std::make_shared<PCSCCardProbe>(this);
    }

    std::tuple<PCSCReaderUnit::SPtrStringVector, PCSCReaderUnit::ReaderStateVector>
    PCSCReaderUnit::prepare_poll_parameters()
    {
        size_t readers_count = 0;
        if (!getName().empty())
        {
            // use dedicated reader
            if (Settings::getInstance()->SeeWaitInsertionLog)
            {
                LOG(LogLevel::INFOS) << "Use specific reader: " << getName() << ".";
            }
            readers_count = 1;
        }
        else
        {
            readers_count = getReaderProvider()->getReaderList().size();
        }

        EXCEPTION_ASSERT_WITH_LOG(readers_count > 0, CardException,
                                  EXCEPTION_MSG_NOREADER);

        SPtrStringVector readers_names(readers_count);
        ReaderStateVector readers(readers_count);
        std::memset(&readers[0], 0, readers.size() * sizeof(SCARD_READERSTATE));

        if (!getName().empty())
        {
            readers_names[0] = std::make_shared<std::string>(getName());
            readers[0].dwCurrentState = SCARD_STATE_UNAWARE;
            readers[0].dwEventState = SCARD_STATE_UNAWARE;
            readers[0].szReader = readers_names[0]->c_str();
        }
        else
        {
            for (size_t i = 0; i < readers_count; ++i)
            {
                readers_names[i] = std::make_shared<std::string>(getReaderProvider()
                                                                     ->getReaderList().at(i)->getName());
                readers[i].dwCurrentState = SCARD_STATE_UNAWARE;
                readers[i].dwEventState = SCARD_STATE_UNAWARE;
                readers[i].szReader = readers_names[i]->c_str();
            }
        }
        return std::make_tuple(readers_names, readers);
    }

    void PCSCReaderUnit::waitInsertion_create_proxy(
        const std::string &reader_name)
    {
        assert(!reader_name.empty());

        std::shared_ptr<PCSCReaderUnitConfiguration> pcscRUC = getPCSCConfiguration();
        if (this->getPCSCType() == PCSC_RUT_DEFAULT)
        {
            d_proxyReaderUnit = PCSCReaderUnit::createPCSCReaderUnit(reader_name);
            if (d_proxyReaderUnit->getPCSCType() == PCSC_RUT_DEFAULT)
            {
                d_proxyReaderUnit.reset();
                d_connectedName = reader_name;
            }
            else
            {
                d_proxyReaderUnit->makeProxy(std::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);
                d_proxyReaderUnit->d_sam_chip = d_sam_chip;
                d_proxyReaderUnit->d_sam_readerunit = d_sam_readerunit;
            }
        }
        else
        {
            // Already a proxy from serialization maybe, just copy instance information to it.
            if (d_proxyReaderUnit)
            {
                d_proxyReaderUnit->makeProxy(std::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);
            }
        }
    }

bool PCSCReaderUnit::process_insertion(const std::string &cardType,
                                       int maxwait,
                                       const ElapsedTimeCounter &elapsed)
{
    if (d_proxyReaderUnit)
        return d_proxyReaderUnit->process_insertion(cardType, maxwait, elapsed);

	d_insertedChip = createChip((d_card_type == CHIP_UNKNOWN) ? cardType : d_card_type);
	if (d_card_type == CHIP_GENERICTAG && cardType != CHIP_UNKNOWN && cardType != CHIP_GENERICTAG)
	{
		std::dynamic_pointer_cast<GenericTagChip>(d_insertedChip)->setRealChip(createChip(cardType));
	}
    if (d_proxyReaderUnit)
    {
        d_proxyReaderUnit->setSingleChip(d_insertedChip);
    }
    return true;
}

std::shared_ptr<Chip> PCSCReaderUnit::adjustChip(std::shared_ptr<Chip> c)
{
    // DESFire adjustment. Check maybe it's DESFireEV1. Check random uid.
    // Adjust cryptographic context.
	if (c->getCardType() == CHIP_DESFIRE && d_card_type == CHIP_UNKNOWN)
    {
        if (createCardProbe()->is_desfire_ev1())
			c = createChip(CHIP_DESFIRE_EV1);
    }
	if (c->getCardType() == CHIP_DESFIRE || c->getCardType() == CHIP_DESFIRE_EV1)
    {
        ByteVector uid;
        if (createCardProbe()->has_desfire_random_uid(&uid))
        {
            c->setChipIdentifier(getCardSerialNumber()); // Has random, cannot rely on get_version
            std::dynamic_pointer_cast<DESFireChip>(c)->setHasRealUID(false);
        }
        else
            c->setChipIdentifier(uid);

        std::dynamic_pointer_cast<DESFireChip>(c)->getCrypto()
            ->setIdentifier(c->getChipIdentifier());
        std::dynamic_pointer_cast<DESFireChip>(c)->getCrypto()
            ->setCryptoContext(c->getChipIdentifier());
    }

	// Mifare Ultralight adjustement.
	if (c->getCardType() == "MifareUltralight" && d_card_type == CHIP_UNKNOWN)
	{
		if (createCardProbe()->is_mifare_ultralight_c())
			c = createChip("MifareUltralightC");
	}

    if (c->getChipIdentifier().size() == 0)
    {
        try
        {
            if (c->getCardType() == CHIP_PROX)
            {
                if (atr_.size() > 2)
                {
                    c->setChipIdentifier(atr_);
                }
            }
            else if (d_card_type == "UNKNOWN" && c && c->getCardType() == "SAM_AV2")
            {
                if (c->getCardType() == "SAM_AV2")
                {
                    if (std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(c->getCommands())->getSAMTypeFromSAM() == "SAM_AV1")
                    {
                        LOG(LogLevel::INFOS) << "SAM on the reader is AV2 but mode AV1 so we switch to AV1.";
                        c = createChip("SAM_AV1");
                    }
                }
            }
            else
            {
                c->setChipIdentifier(getCardSerialNumber());
            }
        }
        catch (LibLogicalAccessException& e)
        {
            LOG(LogLevel::ERRORS) << "Exception while getting card serial number {" << e.what() << "}";
            c->setChipIdentifier(ByteVector());
        }
    }
   return c;
}
}
