/**
 * \file MifareSmartIDReaderCardAdapter.cpp
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Mifare SmartID reader/card adapter.
 */

#include "MifareSmartIDReaderCardAdapter.h"

#include <cstring>

namespace LOGICALACCESS
{	
	std::vector<unsigned char> MifareSmartIDReaderCardAdapter::sendCommand(unsigned char cmd, const std::vector<unsigned char>& command, long int timeout)
	{
		std::vector<unsigned char> wrappedCommand;

		wrappedCommand.push_back(cmd);
		wrappedCommand.push_back(static_cast<unsigned char>(command.size()));
		wrappedCommand.insert(wrappedCommand.end(), command.begin(), command.end());

		std::vector<unsigned char> result;
		result = SmartIDReaderCardAdapter::sendCommand(0x80, wrappedCommand, timeout);

		return result;
	}	

	std::vector<unsigned char> MifareSmartIDReaderCardAdapter::request()
	{
		std::vector<unsigned char> data;
		data.push_back(static_cast<unsigned char>(0x00));

		return sendCommand(0x41, data);
	}

	std::vector<unsigned char> MifareSmartIDReaderCardAdapter::anticollision()
	{
		std::vector<unsigned char> data;
		data.resize(5, 0x00);

		return sendCommand(0x42, data);
	}

	void MifareSmartIDReaderCardAdapter::selectIso(const std::vector<unsigned char>& uid)
	{
		select(uid);
	}

	void MifareSmartIDReaderCardAdapter::select(const std::vector<unsigned char>& uid)
	{
		sendCommand(0x43, uid);
	}

	void MifareSmartIDReaderCardAdapter::halt()
	{
		sendCommand(0x45, std::vector<unsigned char>());
	}
}
