/**
 * \file A3MLGM5600LCDDisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A3MLGM5600 LCD Display.
 */

#include "A3MLGM5600LCDDisplay.h"

namespace logicalaccess
{
	A3MLGM5600LCDDisplay::A3MLGM5600LCDDisplay()
	{
		
	}

	void A3MLGM5600LCDDisplay::setMessage(std::string message)
	{
		for (unsigned char i = 0; i < 2 && (i*15) < static_cast<unsigned char>(message.size()); i ++)
		{
			setMessage(i, message.substr(i*15, (i+1)*15));
		}
	}

	void A3MLGM5600LCDDisplay::setMessage(unsigned char rowid, std::string message)
	{
		std::vector<unsigned char> data;
		data.push_back(0x0f);	// data length
		data.push_back((rowid == 0) ? 0x00 : 0x40);	// row	
		data.insert(data.end(), message.begin(), message.end());
		if (data.size() > 17)
		{
			data = std::vector<unsigned char>(data.begin(), data.begin() + 17);
		}
		else if( data.size() < 17)
		{
			size_t pad = 17 - data.size();
			for (size_t i = 0; i < pad; ++i)
			{
				data.push_back(' ');
			}
		}

		getA3MLGM5600ReaderCardAdapter()->sendCommand(0x14, data);
	}
}
