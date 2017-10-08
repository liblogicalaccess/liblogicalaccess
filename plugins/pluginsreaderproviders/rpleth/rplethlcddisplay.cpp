/**
 * \file rplethlcddisplay.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Rpleth LCD Display.
 */

#include "rplethlcddisplay.hpp"
#include "readercardadapters/rplethreadercardadapter.hpp"

namespace logicalaccess
{
    RplethLCDDisplay::RplethLCDDisplay()
    {
    }

    void RplethLCDDisplay::setMessage(std::string message)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(LCD));
        command.push_back(static_cast<unsigned char>(DISPLAY));
        command.push_back(static_cast<unsigned char>(message.size()));
        command.insert(command.end(), message.begin(), message.end());
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLCDDisplay::setMessage(std::string message, int time)
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(LCD));
        command.push_back(static_cast<unsigned char>(DISPLAYT));
        command.push_back(static_cast<unsigned char>(message.size() + 1));
        command.insert(command.end(), message.begin(), message.end());
        command.push_back(static_cast<unsigned char>(time));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLCDDisplay::setMessage(unsigned char /*rowid*/, std::string message)
    {
        setMessage(message);
    }

    void RplethLCDDisplay::setDisplayTime(int time) const
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(LCD));
        command.push_back(static_cast<unsigned char>(DISPLAYTIME));
        command.push_back(static_cast<unsigned char>(0x01));
        command.push_back(static_cast<unsigned char>(time));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLCDDisplay::blink() const
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(LCD));
        command.push_back(static_cast<unsigned char>(BLINK));
        command.push_back(static_cast<unsigned char>(0x00));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLCDDisplay::autoScroll() const
    {
        ByteVector command;
        command.push_back(static_cast<unsigned char>(LCD));
        command.push_back(static_cast<unsigned char>(SCROLL));
        command.push_back(static_cast<unsigned char>(0x00));
        getRplethReaderCardAdapter()->sendRplethCommand(command, false);
    }

    void RplethLCDDisplay::clear()
    {

    }
}