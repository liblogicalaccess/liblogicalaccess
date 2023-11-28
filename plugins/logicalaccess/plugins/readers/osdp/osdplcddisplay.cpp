/**
* \file osdplcddisplay.cpp
* \author Maxime C. <maxime@leosac.com>
* \brief OSDP LCD Display.
*/

#include <logicalaccess/plugins/readers/osdp/osdplcddisplay.hpp>

namespace logicalaccess
{
OSDPLCDDisplay::OSDPLCDDisplay()
{
}

void OSDPLCDDisplay::setMessage(std::string message)
{
    LLA_LOG_CTX("OSDP SetMessage");
    clear();

    for (unsigned char i = 0;
         i < 2 && (i * OSDP_CMD_TEXT_MAX_LEN) < static_cast<unsigned char>(message.size()); i++)
    {
        setMessage(i, message.substr(i * OSDP_CMD_TEXT_MAX_LEN, (i + 1) * OSDP_CMD_TEXT_MAX_LEN));
    }
}

void OSDPLCDDisplay::setMessage(unsigned char rowid, std::string message)
{
    ByteVector data;

    data.insert(data.end(), message.begin(), message.end());
    // Make sure we don't overflow the LCD line
    if (data.size() > OSDP_CMD_TEXT_MAX_LEN)
    {
        data = ByteVector(data.begin(), data.begin() + OSDP_CMD_TEXT_MAX_LEN);
    }
    /*else // or underflow it (resulting in potential garbage on screen)
    {
        while (data.size() < OSDP_CMD_TEXT_MAX_LEN)
        {
            data.push_back(' ');
        }
    }*/
    
    s_text_cmd text;
    text.reader = getOSDPReaderCardAdapter()->getRS485Address();
    text.controlCode = 0x00;
    text.tempTime = 0;
    text.offsetRow = rowid;
    text.offsetCol = 0;
    text.length = static_cast<unsigned char>(data.size());
    memcpy(text.data, &data[0], data.size());

    getOSDPReaderCardAdapter()->getOSDPCommands()->text(text);
}

void OSDPLCDDisplay::clear()
{
    LLA_LOG_CTX("OSDP Clearing LCD screen ");
    // Send "spaces" ?
}
}