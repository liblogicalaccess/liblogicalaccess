/**
 * \file pcscreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC reader unit.
 */

#include "pcscreaderunit.hpp"


#include <iostream>
#include <iomanip>
#include <sstream>

#include "pcscreaderprovider.hpp"
#include "logicalaccess/services/accesscontrol/cardsformatcomposite.hpp"
#include "logicalaccess/cards/chip.hpp"
#include "logicalaccess/dynlibrary/librarymanager.hpp"
#include "logicalaccess/dynlibrary/idynlibrary.hpp"

#include "commands/samav1iso7816commands.hpp"
#include "commands/samav2iso7816commands.hpp"
#include "commands/desfireev1iso7816commands.hpp"
#include "commands/mifarepcsccommands.hpp"
#include "commands/mifarescmcommands.hpp"
#include "commands/mifarecherrycommands.hpp"
#include "commands/mifarespringcardcommands.hpp"
#include "commands/iso15693pcsccommands.hpp"
#include "commands/iso7816iso7816commands.hpp"
#include "commands/twiciso7816commands.hpp"
#include "commands/mifareultralightpcsccommands.hpp"
#include "commands/mifareultralightcpcsccommands.hpp"
#include "commands/mifareomnikeyxx27commands.hpp"
#include "readercardadapters/pcscreadercardadapter.hpp"
#include "mifareplussl1profile.hpp"
#include "commands/mifareplusspringcardcommandssl1.hpp"
#include "mifareplussl3profile.hpp"
#include "commands/mifareplusspringcardcommandssl3.hpp"
#include "samav1chip.hpp"

#include "commands/samiso7816resultchecker.hpp"
#include "commands/desfireiso7816resultchecker.hpp"
#include "commands/mifareomnikeyxx27resultchecker.hpp"

#include "readers/omnikeyxx21readerunit.hpp"
#include "readers/omnikeylanxx21readerunit.hpp"
#include "readers/omnikeyxx25readerunit.hpp"
#include "readers/omnikeyxx27readerunit.hpp"
#include "readers/cherryreaderunit.hpp"
#include "readers/scmreaderunit.hpp"
#include "readers/springcardreaderunit.hpp"

#include "pcscdatatransport.hpp"

#include "desfirechip.hpp"
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#ifdef UNIX
#include <sys/time.h>
#endif

namespace logicalaccess
{
	PCSCReaderUnit::PCSCReaderUnit(const std::string& name)
		: ISO7816ReaderUnit()
	{
		d_name = name;
		d_connectedName = d_name;
		d_sch = 0;
		d_share_mode = SM_SHARED;
		d_ap = 0;
		d_atrLength = 0;


		d_card_type = "UNKNOWN";

		try
		{
			boost::property_tree::ptree pt;
			read_xml((boost::filesystem::current_path().string() + "/PCSCReaderUnit.config"), pt);
			d_card_type = pt.get("config.cardType", "UNKNOWN");
		}
		catch (...) { }

		d_proxyReaderUnit.reset();
		d_readerUnitConfig.reset(new PCSCReaderUnitConfiguration());
		setDefaultReaderCardAdapter (boost::shared_ptr<PCSCReaderCardAdapter> (new PCSCReaderCardAdapter()));
	}

	PCSCReaderUnit::~PCSCReaderUnit()
	{
		INFO_SIMPLE_("Reader unit destruction...");

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
		return d_sch;
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
		boost::shared_ptr<PCSCReaderUnitConfiguration> pcscRUC = getPCSCConfiguration();
		PCSCReaderUnitType pcscType = getPCSCType();
		d_proxyReaderUnit = PCSCReaderUnit::createPCSCReaderUnit(d_name);
		if (d_proxyReaderUnit->getPCSCType() == pcscType)
		{
			d_proxyReaderUnit.reset();
		}
		else
		{
			d_proxyReaderUnit->makeProxy(boost::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);								
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

	boost::shared_ptr<PCSCReaderUnit> PCSCReaderUnit::createPCSCReaderUnit(std::string& readerName)
	{
		boost::shared_ptr<PCSCReaderUnit> unit;
		if (readerName.find(string("OMNIKEY")) != string::npos)
		{
			if (readerName.find(string("x21")) != string::npos || readerName.find(string("5321")) != string::npos || readerName.find(string("6321")) != string::npos)
			{
				if (readerName.find(string("LAN")) != string::npos)
				{
					unit.reset(new OmnikeyLANXX21ReaderUnit(readerName));
				}
				else
				{
					unit.reset(new OmnikeyXX21ReaderUnit(readerName));
				}
			}
			else if (readerName.find(string("x25")) != string::npos || readerName.find(string("5025-CL")) != string::npos)
			{
				unit.reset(new OmnikeyXX25ReaderUnit(readerName));
			}
			else if(readerName.find(string("x27")) != string::npos || readerName.find(string("5127")) != string::npos || readerName.find(string("5427")) != string::npos)
			{
				unit.reset(new OmnikeyXX27ReaderUnit(readerName));
			}
		}
		else if (readerName.find(string("SDI010 Contactless Reader")) != string::npos
			  || readerName.find(string("SCR331-DI USB ContactlessReader")) != string::npos
			  || readerName.find(string("SCL010 Contactless")) != string::npos)
		{
			unit.reset(new SCMReaderUnit(readerName));
		}
		else if (readerName.find(string("Cherry ")) != string::npos)
		{
			unit.reset(new CherryReaderUnit(readerName));
		}
		else if (readerName.find(string("SpringCard")) != string::npos)
		{
			unit.reset(new SpringCardReaderUnit(readerName));
		}

		if (!unit)
		{
			unit.reset(new PCSCReaderUnit(readerName));
		}

		return unit;
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

		return (d_sch != 0);
	}

	bool PCSCReaderUnit::waitInsertion(unsigned int maxwait)
	{
		if (isConnected())
		{
			ERROR_SIMPLE_(EXCEPTION_MSG_CONNECTED);
			disconnect();
		}

		if (Settings::getInstance()->SeeWaitInsertionLog)
		{
			INFO_SIMPLE_("Waiting card insertion...");
		}

		LONG r = 0;
		bool usePnp = true;
		int readers_count = 0;

		string reader = getName();
		string connectedReader = "";
		std::string cardType = "UNKNOWN";

		if (reader != "")
		{
			if (Settings::getInstance()->SeeWaitInsertionLog)
			{
				INFO_("Use specific reader: %s.", reader.c_str());
			}
			readers_count = 1;
			usePnp = false;
		}
		else
		{
			if (Settings::getInstance()->SeeWaitInsertionLog)
			{
				INFO_SIMPLE_("Listening on all readers");
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
					INFO_SIMPLE_("No PnP support.");
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

				} else
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

#ifndef _WINDOWS
				struct timeval tv1, tv2;
				struct timezone tz;

				gettimeofday(&tv1, &tz);
#else
				DWORD tc = GetTickCount();
#endif

				do
				{
					loop = false;
					r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), ((maxwait == 0) ? INFINITE : maxwait), readers, readers_count);

					if (SCARD_S_SUCCESS == r)
					{
#ifndef _WINDOWS
						gettimeofday(&tv2, &tz);

						if ((maxwait == 0) || static_cast<unsigned int>((tv2.tv_sec - tv1.tv_sec) * 1000L + (tv2.tv_usec - tv1.tv_usec) / 1000L) < maxwait)
						{
							loop = true;
						}
#else
						DWORD ltc = GetTickCount();

						if ((maxwait == 0) || (ltc - tc) < maxwait)
						{
							loop = true;
						}
#endif

						for (int i = 0; i < readers_count; ++i)
						{
							if ((SCARD_STATE_CHANGED & readers[i].dwEventState) != 0)
							{
								readers[i].dwCurrentState = readers[i].dwEventState;
								if ((SCARD_STATE_PRESENT & readers[i].dwEventState) != 0)
								{
									loop  = false;
									cardType = getCardTypeFromATR(readers[i].rgbAtr, readers[i].cbAtr);
									if (cardType == "UNKNOWN")
									{
										cardType = getGenericCardTypeFromATR(readers[i].rgbAtr, readers[i].cbAtr);
									}
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
								ERROR_("Cannot get status change: %x.", r);
							}
							PCSCDataTransport::CheckCardError(r);
						}
					}
				} while(loop);

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
			boost::shared_ptr<PCSCReaderUnitConfiguration> pcscRUC = getPCSCConfiguration();
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
					d_proxyReaderUnit->makeProxy(boost::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);
					d_proxyReaderUnit->d_sam_chip = d_sam_chip;
					d_proxyReaderUnit->d_sam_readerunit = d_sam_readerunit;
				}
			}
			else
			{
				// Already a proxy from serialization maybe, just copy instance information to it.
				if (d_proxyReaderUnit)
				{
					d_proxyReaderUnit->makeProxy(boost::dynamic_pointer_cast<PCSCReaderUnit>(shared_from_this()), pcscRUC);
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
					d_proxyReaderUnit.reset();
				}
				return true;
			}

			return false;
		}

		if (isConnected())
		{
			THROW_EXCEPTION_WITH_LOG(CardException, EXCEPTION_MSG_CONNECTED);
		}

		if (Settings::getInstance()->SeeWaitRemovalLog)
		{
			INFO_SIMPLE_("Waiting card removal...");
		}

		string reader = getConnectedName();

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
		} else
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
					reader = string(reinterpret_cast<const char*>(readers[i].szReader), strlen(reinterpret_cast<const char*>(readers[i].szReader)));
					break;
				}
			}

			if (reader == "")
			{
				for (int i = 0; i < readers_count; ++i)
				{
					readers[i].dwCurrentState = readers[i].dwEventState;
				}

				bool loop = true;
#ifndef _WINDOWS
				struct timeval tv1, tv2;
				struct timezone tz;

				gettimeofday(&tv1, &tz);
#else
				DWORD tc = GetTickCount();
#endif
				while (loop)
				{
					loop = false;

					r = SCardGetStatusChange(getPCSCReaderProvider()->getContext(), ((maxwait == 0) ? INFINITE : maxwait), readers, readers_count);

					if (SCARD_S_SUCCESS == r)
					{
#ifndef _WINDOWS
						gettimeofday(&tv2, &tz);

						if ((maxwait == 0) || static_cast<unsigned int>((tv2.tv_sec - tv1.tv_sec) * 1000L + (tv2.tv_usec - tv1.tv_usec) / 1000L) < maxwait)
						{
							loop = true;
						}
#else
						DWORD ltc = GetTickCount();

						if ((maxwait == 0) || (ltc - tc) < maxwait)
						{
							loop = true;
						}
#endif
						for (int i = 0; i < readers_count; ++i)
						{
							if ((SCARD_STATE_PRESENT & readers[i].dwEventState) == 0)
							{
								loop = false;

								reader = string(reinterpret_cast<const char*>(readers[i].szReader), strlen(reinterpret_cast<const char*>(readers[i].szReader)));
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
								ERROR_("Cannot get status change: %x.", r);
							}
							PCSCDataTransport::CheckCardError(r);
						}
					}
				}
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

		return (!reader.empty());
	}

	bool PCSCReaderUnit::connect()
	{
		return connect(getPCSCConfiguration()->getShareMode());
	}

	bool PCSCReaderUnit::connect(PCSCShareMode share_mode)
	{
		INFO_("Connecting to the chip... Shared mode {%d}", share_mode);
		bool ret = false;
		if (d_proxyReaderUnit)
		{
			INFO_SIMPLE_("Need to use a proxy reader !");
			ret = d_proxyReaderUnit->connect(share_mode);
			if (ret)
			{
				d_sch = d_proxyReaderUnit->getHandle();
				d_ap = d_proxyReaderUnit->getActiveProtocol();
				d_share_mode = share_mode;
			}
		}
		else
		{
			if (isConnected())
			{
				disconnect();
			}

			LONG lReturn = SCardConnect(getPCSCReaderProvider()->getContext(), reinterpret_cast<const char*>(getConnectedName().c_str()), share_mode, getPCSCConfiguration()->getTransmissionProtocol(), &d_sch, &d_ap);
			if (SCARD_S_SUCCESS == lReturn)
			{
				INFO_SIMPLE_("SCardConnect Success !");

				d_share_mode = share_mode;
				if (d_insertedChip->getChipIdentifier().size() == 0)
				{
					try
					{
						if (d_insertedChip->getCardType() == "Prox")
						{
							if (d_atrLength > 2)
							{
								d_insertedChip->setChipIdentifier(std::vector<unsigned char>(d_atr, d_atr + d_atrLength));
							}
						}
						// Specific behavior for DESFire to check if it is not a DESFire EV1
						else if (d_card_type == "UNKNOWN" && d_insertedChip)
						{
							if (d_insertedChip->getCardType() == "DESFire")
							{
								try
								{
	#ifdef _WINDOWS
					Sleep(100);
	#elif defined(__unix__)
					usleep(100000);
	#endif
									DESFireCommands::DESFireCardVersion cardversion;
									boost::dynamic_pointer_cast<DESFireChip>(d_insertedChip)->getDESFireCommands()->getVersion(cardversion);
									// Set from the version

									// DESFire EV1 and not regular DESFire
									if (cardversion.softwareMjVersion >= 1)
									{
										d_insertedChip = createChip("DESFireEV1");
									}
									d_insertedChip->setChipIdentifier(std::vector<unsigned char>(cardversion.uid, cardversion.uid + sizeof(cardversion.uid)));
								}
								catch(std::exception&)
								{
									// Doesn't care about bad communication here, stay DESFire.
								}
							}
							else if (d_insertedChip->getCardType() == "SAM_AV2")
							{
								if (boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(d_insertedChip->getCommands())->getSAMTypeFromSAM() == "SAM_AV1")
								{
									INFO_SIMPLE_("SAM on the reader is AV2 but mode AV1 so we switch to AV1.");
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
						ERROR_("Exception while getting card serial number {%s}", e.what());
						d_insertedChip->setChipIdentifier(std::vector<unsigned char>());
					}
				}

				if (d_insertedChip->getGenericCardType() == "DESFire")
					boost::dynamic_pointer_cast<DESFireISO7816Commands>(d_insertedChip->getCommands())->getCrypto()->setCryptoContext(boost::dynamic_pointer_cast<DESFireProfile>(d_insertedChip->getProfile()), d_insertedChip->getChipIdentifier());
				ret = true;
			}
			else
			{
				INFO_("SCardConnect ERROR {%l} !", lReturn);
			}
		}

		if (ret)
		{
			// Unknown T=CL card, we have to send specific reader command to determine the type (A or B ?)
			if (d_insertedChip->getCardType() == "GENERIC_T_CL")
			{
				INFO_SIMPLE_("Inserted chip is {CT_GENERIC_T_CL}. Looking for type A or B...");
				if (d_proxyReaderUnit)
				{
					setSingleChip(d_proxyReaderUnit->getSingleChip());
				}
				else
				{
					bool isTypeA = false;
					bool isTypeB = false;
					getT_CL_ISOType(isTypeA, isTypeB);

					boost::shared_ptr<Chip> newChip;
					if (isTypeA)
					{
						INFO_SIMPLE_("Type A detected !");
						newChip = createChip("GENERIC_T_CL_A");					
					}
					else if(isTypeB)
					{
						INFO_SIMPLE_("Type B detected !");
						newChip = createChip("GENERIC_T_CL_B");
					}

					if (newChip)
					{
						newChip->setChipIdentifier(d_insertedChip->getChipIdentifier());
						d_insertedChip = newChip;					
					}
					else
					{
						INFO_SIMPLE_("Unable to find the type !");
					}
				}
			}
		}
		else
		{
			d_sch = 0;
			d_ap = 0;
		}

		return ret;
	}

	bool PCSCReaderUnit::reconnect()
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->reconnect();
		}

		if (!isConnected())
		{
			return false;
		}

		if (SCARD_S_SUCCESS == SCardReconnect(d_sch, d_share_mode, getPCSCConfiguration()->getTransmissionProtocol(), SCARD_LEAVE_CARD, &d_ap))
		{
			return true;
		}

		return false;
	}

	void PCSCReaderUnit::disconnect()
	{
		if (d_insertedChip && d_insertedChip->getGenericCardType() == CHIP_SAM)
		{
			disconnect(SCARD_UNPOWER_CARD);
		}
		else
		{
			disconnect(SCARD_LEAVE_CARD);
		}
	}

	void PCSCReaderUnit::disconnect(unsigned int action)
	{
		INFO_SIMPLE_("Disconnecting from the chip.");

		if (d_proxyReaderUnit)
		{
			d_proxyReaderUnit->disconnect();
		}
		else
		{
			if (isConnected())
			{			
				SCardDisconnect(d_sch, action);
			}
		}

		d_sch = 0;
		d_ap = 0;
	}

	bool PCSCReaderUnit::connectToReader()
	{
		INFO_SIMPLE_("Connecting to reader...");

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

				boost::shared_ptr<PCSCReaderUnit> ret;

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

				INFO_SIMPLE_("Checking SAM backward...");

				//Check Backward AV1 => AV2 is active
				if (getPCSCConfiguration()->getSAMType() != "SAM_AUTO" && ret->getSingleChip()->getCardType() == "SAM_AV2" && getPCSCConfiguration()->getSAMType() != boost::dynamic_pointer_cast<SAMCommands<KeyEntryAV2Information, SETAV2> >(ret->getSingleChip()->getCommands())->getSAMTypeFromSAM())
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "SAM on the reader is not the same type as selected.");

				INFO_SIMPLE_("SAM backward ended.");
			
				setSAMChip(boost::dynamic_pointer_cast<SAMChip>(ret->getSingleChip()));
				setSAMReaderUnit(ret);

				INFO_SIMPLE_("Starting SAM Security check...");

				try
				{
					if (getPCSCConfiguration()->getSAMUnLockKey())
					{
						if (ret->getSingleChip()->getCardType() == "SAM_AV1")
							boost::dynamic_pointer_cast<SAMAV1ISO7816Commands>(getSAMChip()->getCommands())->authentificateHost(getPCSCConfiguration()->getSAMUnLockKey(), getPCSCConfiguration()->getSAMUnLockkeyNo());
						else if (ret->getSingleChip()->getCardType() == "SAM_AV2")
							boost::dynamic_pointer_cast<SAMAV2ISO7816Commands>(getSAMChip()->getCommands())->lockUnlock(getPCSCConfiguration()->getSAMUnLockKey(), logicalaccess::SAMLockUnlock::Unlock, getPCSCConfiguration()->getSAMUnLockkeyNo(), 0, 0);
					}
					else
						THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The Unlock SAM key is empty.");

				}
				catch (std::exception&)
				{
					setSAMChip(boost::shared_ptr<SAMChip>());
					setSAMReaderUnit(boost::shared_ptr<PCSCReaderUnit>());
					THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "The SAM Detected is not the SAM waited.");
				}

			}
		return true;
	}

	void PCSCReaderUnit::disconnectFromReader()
	{
		INFO_SIMPLE_("Disconnecting from reader...");

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
				setSAMChip(boost::shared_ptr<SAMChip>());
			}
		}
	}

	boost::shared_ptr<SAMChip> PCSCReaderUnit::getSAMChip()
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->getSAMChip();
		}

		return d_sam_chip;
	}

	void PCSCReaderUnit::setSAMChip(boost::shared_ptr<SAMChip> t)
	{
		if (d_proxyReaderUnit)
		{
			d_proxyReaderUnit->setSAMChip(t);
		}

		d_sam_chip = t;
	}

	boost::shared_ptr<PCSCReaderUnit> PCSCReaderUnit::getSAMReaderUnit()
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->getSAMReaderUnit();
		}

		return d_sam_readerunit;
	}

	void PCSCReaderUnit::setSAMReaderUnit(boost::shared_ptr<PCSCReaderUnit> t)
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
		if (atr != NULL && atrLength >= d_atrLength)
		{
			memcpy(atr, d_atr, d_atrLength);
		}

		return d_atrLength;
	}

	std::string PCSCReaderUnit::getCardTypeFromATR(const unsigned char* atr, size_t atrlen)
	{
		if (atr && (atrlen > 0))
		{
			unsigned char eatr[20] = {
				0x3B, 0x8F, 0x80, 0x01, 0x80,
				0x4F, 0x0C, 0xA0, 0x00, 0x00,
				0x03, 0x06, 0xFF, 0x00, 0xFF,
				0x00, 0x00, 0x00, 0x00, 0xFF
			};

			memcpy(d_atr, reinterpret_cast<const void*>(atr), atrlen);
			d_atrLength = atrlen;

			if (atrlen == 17)
			{
				if ((atr[1] == 0x0F) && (atr[2] == 0xFF))
				{
					switch (atr[16])
					{
						case 0x11:
							{
								return "Mifare1K";
								break;
							}
						case 0x21:
							{
								return "Mifare4K";
								break;
							}
						case 0x93:
							{
								return "HIDiClass2KS";
								break;
							}
						case 0xA3:
							{
								return "HIDiClass16KS";
								break;
							}
						case 0xB3:
							{
								return "HIDiClass8x2KS";
								break;
							}
					}
				} else if ((atr[0] == 0x3B) && (atr[1] == 0xF5))	// Specific Mifare classic stuff (coming from smartcard_list)
				{
					unsigned char atrMifare[] = { 0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91, 0x81, 0x71, 0xFE, 0x40, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x05};
					unsigned char atrMifare2[] = { 0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91, 0x81, 0x71, 0xFE, 0x40, 0x00, 0x41, 0x08, 0x00, 0x00, 0x00, 0x0D};
					unsigned char atrMifare3[] = { 0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91, 0x81, 0x71, 0xFE, 0x40, 0x00, 0x41, 0x18, 0x00, 0x00, 0x00, 0x1D};
					unsigned char atrMifare4[] = { 0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91, 0x81, 0x71, 0xFE, 0x40, 0x00, 0x41, 0x88, 0x00, 0x00, 0x00, 0x8D};
				
					if (!memcmp(atr, atrMifare, sizeof(atrMifare)))
					{
						// Contactless Mifare Ultralight
						return "MifareUltralight";
					}
					else if (!memcmp(atr, atrMifare2, sizeof(atrMifare2)))
					{
						// Contactless Mifare (Type unknown)
						return "Mifare1K";
					}
					else if (!memcmp(atr, atrMifare3, sizeof(atrMifare3)))
					{
						// Contactless Mifare 4k
						return "Mifare4K";
					}
					else if (!memcmp(atr, atrMifare4, sizeof(atrMifare4)))
					{
						// Contactless Mifare 1k or 4k
						return "Mifare1K";
					}
				}
				return "UNKNOWN";
			} else if (atrlen == 11 && (atr[0] == 0x3B) && (atr[1] == 0x09))	// specific Mifare classic stuff again (coming from smartcard_list)
			{
				unsigned char atrMifare[] = { 0x3B, 0x09, 0x41, 0x04, 0x11, 0xDD, 0x82, 0x2F, 0x00, 0x00, 0x88};
				if (!memcmp(atr, atrMifare, sizeof(atrMifare)))
				{
					// 1k contactless Mifare
					return "Mifare1K";
				}
				return "UNKNOWN";
			}
			else if (atrlen == 28 || atrlen == 27)
			{
				// 3B DF 18 FF 81 F1 FE 43 00 3F 03 83 4D 49 46 41 52 45 20 50 6C 75 73 20 53 41 4D 3B NXP SAM AV2 module
				// 3B DF 18 FF 81 F1 FE 43 00 1F 03 4D 49 46 41 52 45 20 50 6C 75 73 20 53 41 4D 98 Mifare SAM AV2
				unsigned char atrTagITP1[] = { 0x3B, 0xDF, 0x18, 0xFF, 0x81, 0xF1, 0xFE, 0x43, 0x00, 0x3F, 0x03, 0x83, 0x4D, 0x49, 0x46, 0x41, 0x52, 0x45, 0x20, 0x50, 0x6C, 0x75, 0x73, 0x20, 0x53, 0x41, 0x4D, 0x3B};
				unsigned char atrTagITP2[] = { 0x3B, 0xDF, 0x18, 0xFF, 0x81, 0xF1, 0xFE, 0x43, 0x00, 0x1F, 0x03, 0x4D, 0x49, 0x46, 0x41, 0x52, 0x45, 0x20, 0x50, 0x6C, 0x75, 0x73, 0x20, 0x53, 0x41, 0x4D, 0x98};

				if (atrlen == 28 && !memcmp(atr, atrTagITP1, sizeof(atrTagITP1)))
				{
					return "SAM_AV2";
				}
				else if (atrlen == 27 && !memcmp(atr, atrTagITP2, sizeof(atrTagITP2)))
				{
					return "SAM_AV2";
				}
			}
			else
			{
				if (atrlen == 20)
				{
					memcpy(eatr + 12, atr + 12, 3);
					eatr[19] = atr[19];

					if (memcmp(eatr, atr, atrlen) != 0)
					{
						return "UNKNOWN";
					}
					else
					{
						std::string ret = atrXToCardType((atr[13] << 8) | atr[14]);
						if (ret != "UNKNOWN")
						{
							return ret;
						}

						unsigned char atrTagIT[] = { 0x3b, 0x8f, 0x80, 0x01, 0x80, 0x4f, 0x0c, 0xa0, 0x00, 0x00, 0x03, 0x06, 0x0b, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x71 };
						if (!memcmp(atr, atrTagIT, sizeof(atrTagIT)))
						{
							return"TagIt";
						}
					}
				} else
				{
					if (atrlen == 6)
					{
						unsigned char atrDESFire[] = { 0x3b, 0x81, 0x80, 0x01, 0x80, 0x80 };
						if (!memcmp(atr, atrDESFire, sizeof(atrDESFire)))
						{
							return "DESFire";
						}
					}
					else if (atrlen == 19)
					{
						unsigned char atrDESFire[] = { 0x3b, 0xf7, 0x91, 0x00, 0xff, 0x91, 0x81, 0x71, 0xFE, 0x40, 0x00, 0x41, 0x20, 0x00, 0x11, 0x77, 0x81, 0x80, 0x40};
						if (!memcmp(atr, atrDESFire, sizeof(atrDESFire)))
						{
							return "DESFire";
						}
					}
					else
					{
						if (atrlen > 2)
						{
							if (atr[0] == 0x3B)
							{
								if (atr[1] == 0x05 || atr[1] == 0x06 || atr[1] == 0x07)
								{
									return "Prox";
								}
							}
						}
					}
					return "UNKNOWN";
				}
			}
		}

		return "UNKNOWN";
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

	std::string PCSCReaderUnit::getGenericCardTypeFromATR(const unsigned char* atr, size_t atrlen)
	{
		EXCEPTION_ASSERT_WITH_LOG(atrlen >= 2, LibLogicalAccessException, "The ATR length must be at least 2 bytes long (TS + T0).");

		std::vector<unsigned char> sb(atr, atr + atrlen);
		INFO_("Getting generic card type for ATR %s Len {%d}...", BufferHelper::getHex(sb).c_str(), atrlen);

		std::string cardType = "UNKNOWN";

		//bool isTEqual0Supported = false;
		//bool isTEqual1Supported = false;

		// Check ATR according to PC/SC part3

		unsigned char y = 0;
		unsigned char TS = atr[y++];
		switch (TS)
		{
			case 0x3b:	// Direct convention
			{
				unsigned char T0 = atr[y++];
				unsigned char historicalBytesLength = T0 & 0x0f;
				bool hasTA1 = ((T0 & 0x10) == 0x10);
				if (hasTA1)
				{
					EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TA1.");
					/*unsigned char TA1 = atr[y++]; */
					y++;
					// Analyze TA1 here
				}
				bool hasTB1 = ((T0 & 0x20) == 0x20);
				if (hasTB1)
				{
					EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TB1.");
					/*unsigned char TB1 = atr[y++];*/
					y++;
					// Analyze TB1 here
				}
				bool hasTC1 = ((T0 & 0x40) == 0x40);
				if (hasTC1)
				{
					EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TC1.");
					/*unsigned char TC1 = atr[y++]; */
					y++;
					// Analyze TC1 here
				}
				bool hasTD1 = ((T0 & 0x80) == 0x80);

				if (hasTD1)
				{
					EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TD1.");
					unsigned char TD1 = atr[y++];
					//isTEqual0Supported = (TD1 & 0x0f) == 0;
					bool hasTA2 = ((TD1 & 0x10) == 0x10);
					if (hasTA2)
					{
						EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TA2.");
						/*unsigned char TA2 = atr[y++]; */
						y++;
						// Analyze TA2 here
					}
					bool hasTB2 = ((TD1 & 0x20) == 0x20);
					if (hasTB2)
					{
						EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TB2.");
						/*unsigned char TB2 = atr[y++]; */
						y++;
						// Analyze TB2 here
					}
					bool hasTC2 = ((TD1 & 0x40) == 0x40);
					if (hasTC2)
					{
						EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TC2.");
						/*unsigned char TC2 = atr[y++];*/
						y++;
						// Analyze TC2 here
					}
					bool hasTD2 = ((TD1 & 0x80) == 0x80);

					if (hasTD2)
					{
						EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TD2.");
						unsigned char TD2 = atr[y++];
						//isTEqual1Supported = (TD2 & 0x0f) == 1;
						bool hasTA3 = ((TD2 & 0x10) == 0x10);
						if (hasTA3)
						{
							EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TA3.");
							/*unsigned char TA3 = atr[y++];*/
							y++;
							// Analyze TA3 here
						}
						bool hasTB3 = ((TD2 & 0x20) == 0x20);
						if (hasTB3)
						{
							EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TB3.");
							/*unsigned char TB3 = atr[y++];*/
							y++;
							// Analyze TB3 here
						}
						bool hasTC3 = ((TD2 & 0x40) == 0x40);
						if (hasTC3)
						{
							EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TC3.");
							/*unsigned char TC3 = atr[y++];*/
							y++;
							// Analyze TC3 here
						}
						bool hasTD3 = ((TD2 & 0x80) == 0x80);

						if (hasTD3)
						{
							EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve TD3.");
							/*unsigned char TD3 = atr[y++];*/
							y++;
							// Analyze TD3 here
						}
					}
				}

				// Historical bytes
				if (historicalBytesLength > 0)
				{
					bool isStorageCard = false;

					try
					{				
						unsigned int x = 0;
						EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength, LibLogicalAccessException, "Bad historical buffer. Too short to retrieve Category Indicator.");
						/*unsigned char categoryIndicator = atr[y + x++]; // See ISO7816-4, 8.2*/
						x++;

						EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength, LibLogicalAccessException, "Bad historical buffer. Too short to retrieve Application Identifier Presence Indicator.");
						unsigned char aidIndicator = atr[y + x++];

						EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength, LibLogicalAccessException, "Bad historical buffer. Too short to retrieve length from historical bytes.");
						unsigned char length = atr[y + x++];

						unsigned int w = 0;
						if (aidIndicator == 0x4F)
						{						
							EXCEPTION_ASSERT_WITH_LOG(length >= 5, LibLogicalAccessException, "Bad internal historical buffer. Too short to retrieve the RID.");
							unsigned char pcscRID[5] = {0xA0, 0x00, 0x00, 0x03, 0x06};
							unsigned char rid[5];
							memcpy(rid, &atr[y + x + w], sizeof(rid));
							w += sizeof(rid);

							if (memcmp(pcscRID, rid, sizeof(rid)) == 0)
							{
								EXCEPTION_ASSERT_WITH_LOG(w < length, LibLogicalAccessException, "Bad internal historical buffer. Too short to retrieve the SS.");
								unsigned char SS = atr[y + x + w++];

								switch(SS)
								{
								case 0x09:
								case 0x0a:
								case 0x0b:
								case 0x0c:
									cardType = "ISO15693";
									break;
								}

								EXCEPTION_ASSERT_WITH_LOG(w+1 < length, LibLogicalAccessException, "Bad internal historical buffer. Too short to retrieve the Card Name.");
								unsigned char cardName[2];
								memcpy(cardName, &atr[y + x + w], sizeof(cardName));
								w += sizeof(cardName);

								isStorageCard = true;
							}
						}
					}
					catch(LibLogicalAccessException&)
					{
						isStorageCard = false;
					}

					// The format doesn't match a storage card. It should be a CPU card
					if (!isStorageCard)
					{
						unsigned int x = 0;

						// ISO14443-4 Type B
						// It should be the ATQB without checksum (see ISO14443-3, 7.9 for more information on the format)
						if (historicalBytesLength == 12 && atr[y + x] == 0x50)
						{
							cardType = "GENERIC_T_CL_B";
						}
						else
						{
							unsigned int x = 0;
							EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength, LibLogicalAccessException, "Bad historical buffer. Too short to retrieve Category Indicator.");
							/*unsigned char categoryIndicator = atr[y + x++];	// See ISO7816-4, 8.2*/
							x++;

							cardType = "GENERIC_T_CL";
						}
					}

					y += historicalBytesLength;
				}

				/* TODO: Disabled now because contact smart cards doesn't have a checksum. Need to fix that ! */

				// Check ATR checksum
				/*EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad buffer. Too short to retrieve Checksum.");
				unsigned char calcchecksum = 0x00;
				for (unsigned int i = 1; i < y; ++i)
				{
					calcchecksum ^= atr[i];
				}
				unsigned char checksum = atr[y++];
				EXCEPTION_ASSERT_WITH_LOG(checksum == calcchecksum, LibLogicalAccessException, "Bad buffer. The checksum doesn't match.");*/
			}
			break;
		}		

		INFO_("Generic card type found {%s}...", cardType.c_str());

		return cardType;
	}

	boost::shared_ptr<ReaderCardAdapter> PCSCReaderUnit::getReaderCardAdapter(std::string type)
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->getReaderCardAdapter(type);
		}

		return getDefaultReaderCardAdapter();
	}

	boost::shared_ptr<Chip> PCSCReaderUnit::createChip(std::string type)
	{
		std::string getterName;

		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->createChip(type);
		}

		boost::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
		if (chip)
		{
			INFO_("Chip (%s) created, creating other associated objects...", type.c_str());

			boost::shared_ptr<ReaderCardAdapter> rca = getReaderCardAdapter(type);
			boost::shared_ptr<Commands> commands;
			boost::shared_ptr<ResultChecker> resultChecker(new ISO7816ResultChecker()); // default one

			if (type ==  "Mifare1K" || type == "Mifare4K" || type == "Mifare")
			{
				if (getPCSCType() == PCSC_RUT_SCM_SDI010)
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
					WARNING_SIMPLE_("Cannot found HIDiClass commands.");
				}
			}
			else if (type == "DESFireEV1")
			{
				commands.reset(new DESFireEV1ISO7816Commands());
				boost::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
				resultChecker.reset(new DESFireISO7816ResultChecker());
			}
			else if (type == "DESFire")
			{
				commands.reset(new DESFireISO7816Commands());
				boost::dynamic_pointer_cast<DESFireISO7816Commands>(commands)->setSAMChip(getSAMChip());
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
			else if (type ==  "Twic")
			{
				commands.reset(new TwicISO7816Commands());
			}
			else if (type == "MifareUltralight")
			{
				commands.reset(new MifareUltralightPCSCCommands());
			}
			else if (type == "MifareUltralightC")
			{
				commands.reset(new MifareUltralightCPCSCCommands());
			}
			else if (type == "SAM_AV1")
			{
				commands.reset(new SAMAV1ISO7816Commands());
				boost::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
				boost::dynamic_pointer_cast<SAMAV1ISO7816Commands>(commands)->setCrypto(samcrypto);
				resultChecker.reset(new SAMISO7816ResultChecker());
			}
			else if (type == "SAM_AV2")
			{
				commands.reset(new SAMAV2ISO7816Commands());
				boost::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
				boost::dynamic_pointer_cast<SAMAV2ISO7816Commands>(commands)->setCrypto(samcrypto);
				resultChecker.reset(new SAMISO7816ResultChecker());
			}
			else if (type == "MifarePlus4K")
			{
				//TODO : find the SL without ATR
				std::string ct = getCardTypeFromATR(d_atr, d_atrLength);
				if (ct != "Mifare")
				{
					chip->setProfile(boost::shared_ptr<MifarePlusSL1Profile>(new MifarePlusSL1Profile()));
					if (getPCSCType() == PCSC_RUT_SPRINGCARD)
					{
						commands.reset(new MifarePlusSpringCardCommandsSL1());
					}
				}
				else /* A MODIFIER */
				{
					chip->setProfile(boost::shared_ptr<MifarePlusSL3Profile>(new MifarePlusSL3Profile()));
					if (getPCSCType() == PCSC_RUT_SPRINGCARD)
					{
						commands.reset(new MifarePlusSpringCardCommandsSL3());
					}
				}
			}

			if (type == "DESFire" || type == "DESFireEV1")
			{
				boost::shared_ptr<DESFireISO7816Commands> dcmd = boost::dynamic_pointer_cast<DESFireISO7816Commands>(commands);
				if (dcmd->getSAMChip())
				{
					boost::shared_ptr<SAMDESfireCrypto> samcrypto(new SAMDESfireCrypto());
					if (dcmd->getSAMChip()->getCardType() == "SAM_AV1")
						boost::dynamic_pointer_cast<SAMAV1ISO7816Commands>(dcmd->getSAMChip()->getCommands())->setCrypto(samcrypto);
					else if (dcmd->getSAMChip()->getCardType() == "SAM_AV2")
						boost::dynamic_pointer_cast<SAMAV2ISO7816Commands>(dcmd->getSAMChip()->getCommands())->setCrypto(samcrypto);
				}
			}

			INFO_("Other objects created, making association (ReaderCardAdapter empty? %d - Commands empty? %d)...", !rca, !commands);

			if (rca)
			{
				boost::shared_ptr<DataTransport> dt = getDataTransport();
				if (dt)
				{
					INFO_SIMPLE_("Data transport forced to a specific one.");
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
				boost::shared_ptr<DataTransport> dt = rca->getDataTransport();
				if (dt)
				{
					dt->setResultChecker(resultChecker);
				}
				commands->setReaderCardAdapter(rca);
				commands->setChip(chip);
				chip->setCommands(commands);
			}

			INFO_SIMPLE_("Object creation done.");
		}
		return chip;
	}

	boost::shared_ptr<Chip> PCSCReaderUnit::getSingleChip()
	{
		boost::shared_ptr<Chip> chip;
		if (d_proxyReaderUnit)
			chip = d_proxyReaderUnit->getSingleChip();
		else
			chip = d_insertedChip;
		return chip;
	}

	std::vector<boost::shared_ptr<Chip> > PCSCReaderUnit::getChipList()
	{
		std::vector<boost::shared_ptr<Chip> > chipList;
		boost::shared_ptr<Chip> singleChip = getSingleChip();
		if (singleChip)
		{
			chipList.push_back(singleChip);
		}
		return chipList;
	}

	boost::shared_ptr<PCSCReaderCardAdapter> PCSCReaderUnit::getDefaultPCSCReaderCardAdapter()
	{
		return boost::dynamic_pointer_cast<PCSCReaderCardAdapter>(getDefaultReaderCardAdapter());
	}

	string PCSCReaderUnit::getReaderSerialNumber()
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->getReaderSerialNumber();
		}
		return "";
	}

	boost::shared_ptr<PCSCReaderProvider> PCSCReaderUnit::getPCSCReaderProvider() const
	{
		return boost::dynamic_pointer_cast<PCSCReaderProvider>(getReaderProvider());
	}

	void PCSCReaderUnit::makeProxy(boost::shared_ptr<PCSCReaderUnit> readerUnit, boost::shared_ptr<PCSCReaderUnitConfiguration> readerUnitConfig)
	{
		d_card_type = readerUnit->getCardType();		
		d_readerProvider = readerUnit->getReaderProvider();
		if (getPCSCConfiguration()->getPCSCType() == readerUnitConfig->getPCSCType())
		{
			d_readerUnitConfig = readerUnitConfig;
		}

		d_ap = readerUnit->getActiveProtocol();
		d_share_mode = readerUnit->getShareMode();
		d_atrLength = readerUnit->getATR(d_atr, sizeof(d_atr));
		d_sch = readerUnit->getHandle();
		d_insertedChip = readerUnit->getSingleChip();
	}

	void PCSCReaderUnit::setSingleChip(boost::shared_ptr<Chip> chip)
	{
		d_insertedChip = chip;
	}

	boost::shared_ptr<ReaderUnitConfiguration> PCSCReaderUnit::getConfiguration()
	{
		if (d_proxyReaderUnit)
		{
			return d_proxyReaderUnit->getConfiguration();
		}

		return d_readerUnitConfig;
	}

	void PCSCReaderUnit::setConfiguration(boost::shared_ptr<ReaderUnitConfiguration> config)
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

	void PCSCReaderUnit::changeReaderKey(boost::shared_ptr<ReaderMemoryKeyStorage> keystorage, const std::vector<unsigned char>& key)
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
}
