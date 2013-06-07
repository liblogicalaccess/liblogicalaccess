
/**	
 * \file rplethreadercardadapter.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth reader/card adapter.
 */

#include "iso7816rplethreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

namespace logicalaccess
{		
	ISO7816RplethReaderCardAdapter::ISO7816RplethReaderCardAdapter()
		: ISO7816ReaderCardAdapter()
	{
		d_rpleth_reader_card_adapter.reset(new RplethReaderCardAdapter ());
	}

	ISO7816RplethReaderCardAdapter::~ISO7816RplethReaderCardAdapter()
	{
		
	}

	boost::shared_ptr<ReaderUnit> ISO7816RplethReaderCardAdapter::getReaderUnit()
	{
		return d_rpleth_reader_card_adapter->getReaderUnit();
	}

	void ISO7816RplethReaderCardAdapter::setReaderUnit(boost::weak_ptr<ReaderUnit> unit)
	{
		d_rpleth_reader_card_adapter->setReaderUnit(unit);
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::sendCommand(const std::vector<unsigned char>& command, long int timeout)
	{
		char tmp [3];
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(Device::HID));
		data.push_back(static_cast<unsigned char>(HidCommand::COM));
		data.push_back(static_cast<unsigned char>((command.size()*2)+3));
		data.push_back(static_cast<unsigned char>('t'));
		data.push_back(static_cast<unsigned char>(command.size()));
		data.push_back(0x00);
		for (size_t i = 0; i < command.size(); i++)
		{
			sprintf(tmp, "%.2X", command[i]);
			data.push_back(static_cast<unsigned char>(tmp[0]));
			data.push_back(static_cast<unsigned char>(tmp[1]));
		}
		std::vector<unsigned char> answer = d_rpleth_reader_card_adapter->sendCommand (data, timeout);
		boost::shared_ptr<RplethReaderUnit> readerUnit = boost::dynamic_pointer_cast<RplethReaderUnit>(d_rpleth_reader_card_adapter->getReaderUnit());

		answer = readerUnit->asciiToHex (answer);

		answer = handleAnswer (answer);
		// add check in the answer size (first byte)

		return answer;
	}

	std::vector<unsigned char> ISO7816RplethReaderCardAdapter::handleAnswer (std::vector<unsigned char> answer)
	{
		std::vector<unsigned char> res;
		if (res.size() > 1)
		{
			if (res[0] == res.size()-1)
			{
				res.erase (res.begin());
			}
			else
				res.clear();
		}
		else
		{
			res.clear();
		}

		return res;
	}
}
