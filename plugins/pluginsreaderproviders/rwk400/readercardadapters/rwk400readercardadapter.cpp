/**	
 * \file rwk400readercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rwk400 reader/card adapter.
 */

#include "../commands/desfirerwk400commands.hpp"
#include "rwk400readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	Rwk400ReaderCardAdapter::Rwk400ReaderCardAdapter()
		: ReaderCardAdapter()
	{
	}

	Rwk400ReaderCardAdapter::~Rwk400ReaderCardAdapter()
	{
		
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		COM_ ("SendCommand : %s", BufferHelper::getHex(command).c_str());
		std::vector<unsigned char> res;
		std::vector<unsigned char> cmd;
		boost::shared_ptr<Rwk400ReaderUnit> readerUnit = boost::dynamic_pointer_cast<Rwk400ReaderUnit>(getReaderUnit());
		boost::shared_ptr<SerialPort> port = readerUnit->getSerialPort()->getSerialPort();
		port->write (command);
		try
		{
			receiveAnwser (res, timeout);
		}
		catch(std::invalid_argument&)
		{
			THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "No answer received");
		}
		return res;
	}

	bool Rwk400ReaderCardAdapter::receiveAnwser(std::vector<unsigned char>& buf, long int timeout)
	{	
		#ifdef _WINDOWS
			Sleep(500);
		#elif defined(LINUX)
			usleep(500000);
		#endif
		std::vector<unsigned char> res;
		boost::shared_ptr<SerialPortXml> port = getRwk400ReaderUnit()->getSerialPort();

		boost::posix_time::time_duration ptimeout = boost::posix_time::milliseconds(timeout + 500);
		while (port->getSerialPort()->select(ptimeout))
		{
			std::vector<unsigned char> tmpbuf;
			while (port->getSerialPort()->read(tmpbuf, 256) > 0)
			{
				res.insert(res.end(), tmpbuf.begin(), tmpbuf.end());

				try
				{
					buf = handleAnswerBuffer(res);
					return true;
				}
				catch (std::invalid_argument&)
				{
					THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "No answer received");
				}
			}
		}

		return false;
	}


	std::vector<unsigned char> Rwk400ReaderCardAdapter::handleAnswerBuffer(const std::vector<unsigned char>& ansbuf)
	{
		COM_("Answer brute %s and his size is : %d", BufferHelper::getHex(ansbuf).c_str(), ansbuf.size());
		std::vector<unsigned char> data;
		if (ansbuf.size () == 1)
		{
			EXCEPTION_ASSERT_WITH_LOG(isRWK400Commands (ansbuf[0]) , std::invalid_argument, "Invalid ACK receive");	
			data.push_back (ansbuf[0]);
		}
		else
		{
			EXCEPTION_ASSERT_WITH_LOG(ansbuf.size ()>1, std::invalid_argument, "Anwser size cannot be smaller than 2 bytes");	
			unsigned char sw1 = ansbuf[ansbuf.size()-2];
			unsigned char sw2 = ansbuf[ansbuf.size()-1];
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x67 || sw2 != 0x00 , std::invalid_argument, "P3 incorrect");	
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x6B || sw2 != 0x00 , std::invalid_argument, "P1,P2 Incorrect");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x6D || sw2 != 0x00 , std::invalid_argument, "Invalid instruction");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x6E || sw2 != 0x00 , std::invalid_argument, "Invalid Class");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x69 || sw2 != 0x82 , std::invalid_argument, "Card Not Identified");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x6A || sw2 != 0x82 , std::invalid_argument, "Card Not Found");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x62 || sw2 != 0x00 , std::invalid_argument, "EEPROM Error");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x98 || sw2 != 0x35 , std::invalid_argument, "Command Flow Incorrect");
			EXCEPTION_ASSERT_WITH_LOG(sw1 != 0x3B || sw2 != 0x00 , std::invalid_argument, "Reader Reset");
			EXCEPTION_ASSERT_WITH_LOG(sw1 == 0x90 && sw2 == 0x00 , std::invalid_argument, "Anwser is corrupted");
			if (ansbuf.size() > 3)
			{

				if (isRWK400Commands (ansbuf[1]))
				{
					if (ansbuf.size() > 4)
						data.insert(data.end(), ansbuf.begin()+2, ansbuf.end()-2);
				}
				else
					data.insert(data.end(), ansbuf.begin()+1, ansbuf.end()-2);
			}
		}
		return data;
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::requestA()
	{
		return std::vector<unsigned char>();
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::requestATS()
	{
		getRwk400ReaderUnit()->rats();
		return std::vector<unsigned char>();
	}

	void Rwk400ReaderCardAdapter::haltA()
	{
		/* TODO: Something ? */
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::anticollisionA()
	{
		/* TODO: Something ? */
		return std::vector<unsigned char>();
	}
	
	bool Rwk400ReaderCardAdapter::isRWK400Commands (unsigned char source)
	{
		bool res = false;
		switch (source)
		{
			case RWK400Commands::ASK_RANDOM :	
				res = true;
				break;
			case RWK400Commands::SELECT_CURRENT_KEY :	
				res = true;
				break;
			case RWK400Commands::ENCRYPT_DECRYPT :
				res = true;
				break;
			case RWK400Commands::LED_SOUNDER_CONTROL :	
				res = true;
				break;
			case RWK400Commands::LOAD_KEY :	
				res = true;
				break;
			case RWK400Commands::OPEN_COLLECTOR_OUTPUT :	
				res = true;
				break;
			case RWK400Commands::PAGE_SELECT :	
				res = true;
				break;
			case RWK400Commands::READ_EEPROM :	
				res = true;
				break;
			case RWK400Commands::RESET_RF :	
				res = true;
				break;
			case RWK400Commands::RETREIVE_DATA :	
				res = true;
				break;
			case RWK400Commands::SELECT_CARD :	
				res = true;
				break;
			case RWK400Commands::TRANSMIT :	
				res = true;
				break;
			case RWK400Commands::RATS :	
				res = true;
				break;
			default :
				res = false;
		}
		return res;
	}

	RWK400Commands Rwk400ReaderCardAdapter::toRWK400Commands (unsigned char source)
	{
		RWK400Commands res;
		switch (source)
		{
			case RWK400Commands::ASK_RANDOM :	
				res = RWK400Commands::ASK_RANDOM;
				break;
			case RWK400Commands::SELECT_CURRENT_KEY :	
				res = RWK400Commands::SELECT_CURRENT_KEY;
				break;
			case RWK400Commands::ENCRYPT_DECRYPT :
				res = RWK400Commands::ENCRYPT_DECRYPT;
				break;
			case RWK400Commands::LED_SOUNDER_CONTROL :	
				res = RWK400Commands::LED_SOUNDER_CONTROL;
				break;
			case RWK400Commands::LOAD_KEY :	
				res = RWK400Commands::LOAD_KEY;
				break;
			case RWK400Commands::OPEN_COLLECTOR_OUTPUT :	
				res = RWK400Commands::OPEN_COLLECTOR_OUTPUT;
				break;
			case RWK400Commands::PAGE_SELECT :	
				res = RWK400Commands::PAGE_SELECT;
				break;
			case RWK400Commands::READ_EEPROM :	
				res = RWK400Commands::READ_EEPROM;
				break;
			case RWK400Commands::RESET_RF :	
				res = RWK400Commands::RESET_RF;
				break;
			case RWK400Commands::RETREIVE_DATA :	
				res = RWK400Commands::RETREIVE_DATA;
				break;
			case RWK400Commands::SELECT_CARD :	
				res = RWK400Commands::SELECT_CARD;
				break;
			case RWK400Commands::TRANSMIT :	
				res = RWK400Commands::TRANSMIT;
				break;
			case RWK400Commands::RATS :	
				res = RWK400Commands::RATS;
				break;
			default :
				THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "Bad rwk400 command");
		}
		return res;
	}

	unsigned char Rwk400ReaderCardAdapter::fromRWK400Commands (RWK400Commands source)
	{
		return static_cast<unsigned char>(source);
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::requestB(unsigned char /*afi*/)
	{
		return std::vector<unsigned char>();
	}

	void Rwk400ReaderCardAdapter::haltB()
	{
		haltA();
	}

	void Rwk400ReaderCardAdapter::attrib()
	{
		/* TODO: Something ? */
	}

	std::vector<unsigned char> Rwk400ReaderCardAdapter::anticollisionB(unsigned char /*afi*/)
	{
		/* TODO: Something ? */
		return std::vector<unsigned char>();
	}	

}

