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
#include "commands/mifareultralightcacsacrcommands.hpp"
#include "commands/mifareultralightcspringcardcommands.hpp"
#include "commands/mifareomnikeyxx27commands.hpp"
#include "commands/mifareomnikeyxx21commands.hpp"
#include "commands/mifareplus_omnikeyxx21_sl1.hpp"
#include "commands/mifareplus_sprincard_sl1.hpp"

#include "commands/proxcommand.hpp"
#include "mifareplussl1profile.hpp"
#include "samav1chip.hpp"
#include "commands/felicascmcommands.hpp"

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

#include "../../pluginscards/mifareplus/mifarepluschip.hpp"
#include "../../pluginscards/mifareplus/MifarePlusSL1Chip.hpp"


#include "pcscdatatransport.hpp"

#include "desfirechip.hpp"
#include "../../pluginscards/mifareplus/MifarePlusSL0Commands.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <logicalaccess/utils.hpp>

#include "readers/acsacr1222llcddisplay.hpp"


#include "logicalaccess/settings.hpp"
#include "pcsc_ctl_datatransport.hpp"
#include "commands/mifareplus_acsacr1222l_sl1.hpp"
#include "commands/mifareplus_pcsc_sl3.hpp"
#include "atrparser.hpp"

namespace logicalaccess
{
    PCSCReaderUnit::PCSCReaderUnit(const std::string& name)
        : ISO7816ReaderUnit(), d_name(name), d_connectedName(name)
    {
        d_card_type = "UNKNOWN";

        try
        {
            boost::property_tree::ptree pt;
            read_xml((boost::filesystem::current_path().string() + "/PCSCReaderUnit.config"), pt);
            d_card_type = pt.get("config.cardType", "UNKNOWN");
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
            reader = std::make_shared<PCSCReaderUnit>(readerName);
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
        LONG r = 0;
        bool usePnp = true;
        int readers_count = 0;

        std::string reader = getName();
        std::string connectedReader = "";
        std::string cardType = "UNKNOWN";

        if (reader != "")
        {
            if (Settings::getInstance()->SeeWaitInsertionLog)
            {
                LOG(LogLevel::INFOS) << "Use specific reader: " << reader.c_str() << ".";
            }
            readers_count = 1;
            usePnp = false;
        }
        else
        {
            if (Settings::getInstance()->SeeWaitInsertionLog)
            {
                LOG(LogLevel::INFOS) << "Listening on all readers";
            }
            readers_count = static_cast<int>(getReaderProvider()->getReaderList().size());

            SCARD_READERSTATE rgReaderStates[1];
            rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
            rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;

            r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), 0, rgReaderStates, 1);

            if (rgReaderStates[0].dwEventState & SCARD_STATE_UNKNOWN)
            {
                usePnp = false;
                if (Settings::getInstance()->SeeWaitInsertionLog)
                {
                    LOG(LogLevel::INFOS) << "No PnP support.";
                }
            }
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

        if (readers_names)
        {
            SCARD_READERSTATE* readers = new SCARD_READERSTATE[readers_count + (usePnp ? 1 : 0)];

            if (readers)
            {
                memset(readers, 0x00, sizeof(SCARD_READERSTATE) * (readers_count + (usePnp ? 1 : 0)));

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
                if (usePnp)
                {
                    readers[readers_count].dwCurrentState = SCARD_STATE_UNAWARE;
                    readers[readers_count].dwEventState = SCARD_STATE_UNAWARE;
                    readers[readers_count].szReader = "\\\\?PnP?\\Notification";
                    readers_count++;
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
                            if ((SCARD_STATE_CHANGED & readers[i].dwEventState) != 0)
                            {
                                readers[i].dwCurrentState = readers[i].dwEventState;
                                if ((SCARD_STATE_PRESENT & readers[i].dwEventState) != 0)
                                {
                                    atr_ = std::vector<uint8_t>(readers[i].rgbAtr,
                                                                readers[i].rgbAtr + readers[i].cbAtr);
                                    cardType = ATRParser::guessCardType(atr_, getPCSCType());
                                    loop = false;
                                    LOG(INFOS) << "Guessed card type from atr: " << cardType;
                                    connectedReader = std::string(reinterpret_cast<const char*>(readers[i].szReader));

                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (r != SCARD_E_TIMEOUT)
                        {
                            if (Settings::getInstance()->SeeWaitInsertionLog)
                            {
                                LOG(LogLevel::ERRORS) << "Cannot get status change: " << r << ".";
                            }
                            PCSCDataTransport::CheckCardError(r);
                        }
                    }
                } while (loop);

                if (usePnp)
                {
                    readers_count--;
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
            }

            delete[] readers_names;
            readers_names = NULL;
        }

        if (connectedReader != "")
        {
            std::shared_ptr<PCSCReaderUnitConfiguration> pcscRUC = getPCSCConfiguration();
            if (this->getPCSCType() == PCSC_RUT_DEFAULT)
            {
                d_proxyReaderUnit = PCSCReaderUnit::createPCSCReaderUnit(connectedReader);
                if (d_proxyReaderUnit->getPCSCType() == PCSC_RUT_DEFAULT)
                {
                    d_proxyReaderUnit.reset();
                    d_connectedName = connectedReader;
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

            d_insertedChip = createChip((d_card_type == "UNKNOWN") ? cardType : d_card_type);
            if (d_proxyReaderUnit)
            {
                d_proxyReaderUnit->setSingleChip(d_insertedChip);
            }
        }

        return (connectedReader != "");
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
        LOG(LogLevel::INFOS) << "Connecting to the chip... Shared mode {" << share_mode << "}";
        bool ret = false;
        if (d_proxyReaderUnit)
        {
            LOG(LogLevel::INFOS) << "Need to use a proxy reader !";
            ret = d_proxyReaderUnit->connect(share_mode);
            if (ret)
            {
             //  d_sch = d_proxyReaderUnit->getHandle();
              //  d_ap = d_proxyReaderUnit->getActiveProtocol();
              //  d_share_mode = share_mode;
            }
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
                if (d_insertedChip->getChipIdentifier().size() == 0)
                {
                    try
                    {
                        if (d_insertedChip->getCardType() == "Prox")
                        {
                            if (atr_.size() > 2)
                            {
                                d_insertedChip->setChipIdentifier(atr_);
                            }
                        }
                        // Specific behavior for DESFire to check if it is not a DESFire EV1
                        else if (d_card_type == "UNKNOWN" && d_insertedChip && (d_insertedChip->getCardType() == "DESFire" || d_insertedChip->getCardType() == "SAM_AV2"))
                        {
                            if (d_insertedChip->getCardType() == "DESFire")
                            {
								std::shared_ptr<DESFireChip> insertedChip = std::dynamic_pointer_cast<DESFireChip>(d_insertedChip);
                                EXCEPTION_ASSERT_WITH_LOG(insertedChip, LibLogicalAccessException, "Wrong card type: expected DESFire.");

                                try
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                    DESFireCommands::DESFireCardVersion cardversion;
                                    insertedChip->getDESFireCommands()->selectApplication(0x00);
                                    insertedChip->getDESFireCommands()->getVersion(cardversion);
                                    // Set from the version

                                    // DESFire EV1 and not regular DESFire
                                    if (cardversion.softwareMjVersion >= 1)
                                    {
                                        d_insertedChip = createChip("DESFireEV1");
                                    }
                                    // If random UID is enabled, GetVersion will return a full-zero UID.
                                    // We need to call the classic PCSC GetUID function
                                    if (BufferHelper::allZeroes(cardversion.uid))
                                    {
                                        d_insertedChip->setChipIdentifier(getCardSerialNumber());
                                        insertedChip->setRandomUIDEnabled(true);
                                    }
                                    else
                                    {
                                        d_insertedChip->setChipIdentifier(std::vector<unsigned char>(cardversion.uid, cardversion.uid + sizeof(cardversion.uid)));
                                    }
									std::dynamic_pointer_cast<DESFireISO7816Commands>(d_insertedChip->getCommands())->getCrypto()
										->setIdentifier(d_insertedChip->getChipIdentifier());
                                }
                                catch (std::exception&)
                                {
                                    // Doesn't care about bad communication here, stay DESFire.
                                }
                            }
                            else if (d_insertedChip->getCardType() == "SAM_AV2")
                            {
                                if (std::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2>>(d_insertedChip->getCommands())->getSAMTypeFromSAM() == "SAM_AV1")
                                {
                                    LOG(LogLevel::INFOS) << "SAM on the reader is AV2 but mode AV1 so we switch to AV1.";
                                    d_insertedChip = createChip("SAM_AV1");
                                }
                            }

                            if (d_proxyReaderUnit)
                            {
                                d_proxyReaderUnit->setSingleChip(d_insertedChip);
                            }
                        }
                        else
                        {
                            d_insertedChip->setChipIdentifier(getCardSerialNumber());
                        }
                    }
                    catch (LibLogicalAccessException& e)
                    {
                        LOG(LogLevel::ERRORS) << "Exception while getting card serial number {" << e.what() << "}";
                        d_insertedChip->setChipIdentifier(std::vector<unsigned char>());
                    }
                }

                if (d_insertedChip->getGenericCardType() == "DESFire")
                    std::dynamic_pointer_cast<DESFireISO7816Commands>(d_insertedChip->getCommands())->getCrypto()->setCryptoContext(std::dynamic_pointer_cast<DESFireProfile>(d_insertedChip->getProfile()), d_insertedChip->getChipIdentifier());
                ret = true;
        }

        if (ret)
        {
            // Unknown T=CL card, we have to send specific reader command to determine the type (A or B ?)
            if (d_insertedChip->getCardType() == "GENERIC_T_CL")
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
        if (ret && d_proxyReaderUnit)
            d_proxyReaderUnit->cardConnected();
        else if (ret)
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

        if (getPCSCConfiguration()->getSAMType() != "SAM_NONE" && getPCSCConfiguration()->getSAMReaderName() == "")
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Sam type specified without specifying SAM Reader Name");
        if (getPCSCConfiguration()->getSAMType() != "SAM_NONE")
        {
            if (getReaderProvider()->getReaderList().size() < 2)
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "Not Enough reader on the system to use SAM");

            int i = 0;
            for (; i < static_cast<int>(getReaderProvider()->getReaderList().size()); ++i)
            {
                if (getReaderProvider()->getReaderList()[i]->getName() == getPCSCConfiguration()->getSAMReaderName())
                    break;
            }

            if (i == (int)(getReaderProvider()->getReaderList().size()))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The SAM Reader specified has not been find.");

            std::shared_ptr<PCSCReaderUnit> ret;

            if (d_sam_readerunit && d_sam_readerunit->getName() == getPCSCConfiguration()->getSAMReaderName())
                ret = d_sam_readerunit;
            else
            {
                ret.reset(new PCSCReaderUnit(getPCSCConfiguration()->getSAMReaderName()));
                ret->setReaderProvider(getReaderProvider());
            }
            ret->connectToReader();

            if (!ret->waitInsertion(1))
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "No SAM detected on the reader");

            ret->connect();

            LOG(LogLevel::INFOS) << "Checking SAM backward...";

            std::string SAMType = getPCSCConfiguration()->getSAMType();
            if (SAMType != "SAM_AUTO" && SAMType != ret->getSingleChip()->getCardType())
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAM on the reader is not the same type as selected.");

            LOG(LogLevel::INFOS) << "SAM backward ended.";

            setSAMChip(std::dynamic_pointer_cast<SAMChip>(ret->getSingleChip()));
            setSAMReaderUnit(ret);

            LOG(LogLevel::INFOS) << "Starting SAM Security check...";

            try
            {
				if (!getPCSCConfiguration()->getSAMUnLockKey())
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The Unlock SAM key is empty.");

				if (ret->getSingleChip()->getCardType() == "SAM_AV1")
					std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(getSAMChip()->getCommands())->authentificateHost(getPCSCConfiguration()->getSAMUnLockKey(), getPCSCConfiguration()->getSAMUnLockkeyNo());
				else if (ret->getSingleChip()->getCardType() == "SAM_AV2")
				{
					try
					{
						std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands())->lockUnlock(getPCSCConfiguration()->getSAMUnLockKey(), logicalaccess::SAMLockUnlock::Unlock, getPCSCConfiguration()->getSAMUnLockkeyNo(), 0, 0);
					}
					catch (CardException& ex)
					{
						if (ex.error_code() != CardException::WRONG_P1_P2)
							std::rethrow_exception(std::current_exception());

						//try to lock the SAM in case it was already unlocked
						std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands())->lockUnlock(getPCSCConfiguration()->getSAMUnLockKey(),
							logicalaccess::SAMLockUnlock::LockWithoutSpecifyingKey, getPCSCConfiguration()->getSAMUnLockkeyNo(), 0, 0);

						std::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands())->lockUnlock(getPCSCConfiguration()->getSAMUnLockKey(), logicalaccess::SAMLockUnlock::Unlock, getPCSCConfiguration()->getSAMUnLockkeyNo(), 0, 0);
					}
				}
            }
            catch (std::exception&)
            {
                setSAMChip(std::shared_ptr<SAMChip>());
                setSAMReaderUnit(std::shared_ptr<PCSCReaderUnit>());
                THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The SAM Detected is not the SAM waited.");
            }

			//Set SAM unlock key to the SAM Reader in case of reconnect
			ret->getISO7816Configuration()->setSAMUnlockKey(getPCSCConfiguration()->getSAMUnLockKey(), getPCSCConfiguration()->getSAMUnLockkeyNo());
        }
        return true;
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
            if (d_sam_readerunit)
            {
                d_sam_readerunit->disconnect();
                d_sam_readerunit->disconnectFromReader();
                setSAMChip(std::shared_ptr<SAMChip>());
            }
           teardown_pcsc_connection();
        }
    }

    std::shared_ptr<SAMChip> PCSCReaderUnit::getSAMChip()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getSAMChip();
        }

        return d_sam_chip;
    }

    void PCSCReaderUnit::setSAMChip(std::shared_ptr<SAMChip> t)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setSAMChip(t);
        }

        d_sam_chip = t;
    }

    std::shared_ptr<PCSCReaderUnit> PCSCReaderUnit::getSAMReaderUnit()
    {
        if (d_proxyReaderUnit)
        {
            return d_proxyReaderUnit->getSAMReaderUnit();
        }

        return d_sam_readerunit;
    }

    void PCSCReaderUnit::setSAMReaderUnit(std::shared_ptr<PCSCReaderUnit> t)
    {
        if (d_proxyReaderUnit)
        {
            d_proxyReaderUnit->setSAMReaderUnit(t);
        }

        d_sam_readerunit = t;
    }

    std::vector<unsigned char> PCSCReaderUnit::getCardSerialNumber()
    {
        std::vector<unsigned char> ucReceivedData;

        ucReceivedData = getDefaultPCSCReaderCardAdapter()->sendAPDUCommand(0xFF, 0xCA, 0x00, 0x00, 0x00);

        return std::vector<unsigned char>(ucReceivedData.begin(), ucReceivedData.end() - 2);
    }

    size_t PCSCReaderUnit::getATR(void* atr, size_t atrLength)
    {
        if (atr != nullptr && atrLength >= atr_.size())
        {
            memcpy(atr, &atr_[0], atr_.size());
        }
        return atr_.size();
    }

std::string PCSCReaderUnit::atrXToCardType(int code) const
    {
        switch (code)
        {
        case 0x01:
            return "Mifare1K";
        case 0x02:
            return "Mifare4K";
        case 0x03:
            return "MifareUltralight";
        case 0x11:
            return "DESFire";
        case 0x1A:
            return "HIDiClass16KS";
        case 0x1C:
            return "HIDiClass8x2KS";
        case 0x18:
            return "HIDiClass2KS";
        case 0x1D:
            return "HIDiClass32KS_16_16";
        case 0x1E:
            return "HIDiClass32KS_16_8x2";
        case 0x1F:
            return "HIDiClass32KS_8x2_16";
        case 0x20:
            return "HIDiClass32KS_8x2_8x2";
        case 0x26:
            return "MIFARE_MINI";
        case 0x3A:
            return "MifareUltralightC";

        default:
            return "UNKNOWN";
        }
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

            if (type == "Mifare1K" || type == "Mifare4K" || type == "Mifare")
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
                    commands.reset(new MifareOmnikeyXX27Commands());
                    resultChecker.reset(new MifareOmnikeyXX27ResultChecker());
                }
                else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_LAN_XX21)
                {
                    commands.reset(new MifareOmnikeyXX21Commands());
                }
                else if (getPCSCType() == PCSC_RUT_ACS_ACR_1222L)
                {
                    commands.reset(new MifareACR1222LCommands());
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
            else if (type == "DESFireEV1")
            {
                commands.reset(new DESFireEV1ISO7816Commands());
                std::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
                resultChecker.reset(new DESFireISO7816ResultChecker());
            }
            else if (type == "DESFire")
            {
                commands.reset(new DESFireISO7816Commands());
                std::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
                resultChecker.reset(new DESFireISO7816ResultChecker());
            }
            else if (type == "ISO15693")
            {
                commands.reset(new ISO15693PCSCCommands());
            }
            else if (type == "ISO7816")
            {
                commands.reset(new ISO7816ISO7816Commands());
            }
            else if (type == "TagIt")
            {
                commands.reset(new ISO15693PCSCCommands());
            }
            else if (type == "Twic")
            {
                commands.reset(new TwicISO7816Commands());
            }
            else if (type == "MifareUltralight")
            {
                commands.reset(new MifareUltralightPCSCCommands());
            }
            else if (type == "MifareUltralightC")
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
                else
                {
                    commands.reset(new MifareUltralightCPCSCCommands());
                }
            }
            else if (type == "SAM_AV1")
            {
                commands.reset(new SAMAV1ISO7816Commands());
                std::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
                std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(commands)->setCrypto(samcrypto);
                resultChecker.reset(new SAMISO7816ResultChecker());
            }
            else if (type == "SAM_AV2")
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
            else if (type == "Prox")
            {   // A dummy command, whose only goal is to allow retrieval of the
                // reader unit later on.
                commands.reset(new ProxCommand());
            }
			else if (type == "FeliCa")
			{
				if (getPCSCType() == PCSC_RUT_SCM)
				{
					commands.reset(new FeliCaSCMCommands());
				}
			}

            if (type == "DESFire" || type == "DESFireEV1")
            {
                std::shared_ptr<DESFireISO7816Commands> dcmd = std::dynamic_pointer_cast<DESFireISO7816Commands>(commands);
                if (dcmd->getSAMChip())
                {
                    std::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
                    if (dcmd->getSAMChip()->getCardType() == "SAM_AV1")
                        std::dynamic_pointer_cast<SAMAV1ISO7816Commands>(dcmd->getSAMChip()->getCommands())->setCrypto(samcrypto);
                    else if (dcmd->getSAMChip()->getCardType() == "SAM_AV2")
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

        auto atr_len = readerUnit->getATR(nullptr, 0);
        atr_.resize(atr_len);
        readerUnit->getATR(&atr_[0], atr_.size());
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

    unsigned long PCSCReaderUnit::getActiveProtocol() const
    {
        if (connection_)
            return connection_->active_protocol_;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "No active connection.");
    }

    PCSCShareMode PCSCReaderUnit::getShareMode() const
    {
        if (connection_)
            return connection_->share_mode_;
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "No active connection.");
    }

    void PCSCReaderUnit::setup_pcsc_connection(PCSCShareMode share_mode)
    {
        assert(connection_ == nullptr);
        if (share_mode == SC_DIRECT)
        {
            connection_ = std::unique_ptr<PCSCConnection>(new PCSCConnection(
                SC_DIRECT,
                0, // No protocol
                getPCSCReaderProvider()->getContext(), getConnectedName()));

            auto ctl_data_transport =
                std::make_shared<PCSCControlDataTransport>();
            ctl_data_transport->setReaderUnit(shared_from_this());
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
                        MifareOmnikeyXX27Commands>();
                commands.reset(cmd);
                resultChecker.reset(new MifareOmnikeyXX27ResultChecker());
            }
            else if (getPCSCType() == PCSC_RUT_OMNIKEY_XX21 || getPCSCType() == PCSC_RUT_OMNIKEY_XX21)
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

}
