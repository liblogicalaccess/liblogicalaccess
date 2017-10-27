/**
 * \file springcardresultchecker.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief SpringCard result checker.
 */

#include "springcardresultchecker.hpp"

namespace logicalaccess
{
SpringCardResultChecker::SpringCardResultChecker()
{
    AddCheck(0x6f, 0x01, "No answer received (no card in the field, or card is mute)");
    AddCheck(0x6f, 0x02, "CRC error in card's answer");
    AddCheck(0x6f, 0x04, "Card authentication failed");
    AddCheck(0x6f, 0x05, "Parity error in card's answer");
    AddCheck(0x6f, 0x06, "Invalid card response opcode");
    AddCheck(0x6f, 0x07, "Bad anticollision sequence");
    AddCheck(0x6f, 0x08, "Card's serial number is invalid");
    AddCheck(0x6f, 0x09, "Card or block locked");
    AddCheck(0x6f, 0x0a, "Card operation denied, must be authenticated first");
    AddCheck(0x6f, 0x0b, "Wrong number of bits in card's answer");
    AddCheck(0x6f, 0x0c, "Wrong number of bytes in card's answer");
    AddCheck(0x6f, 0x0d, "Card counter error");
    AddCheck(0x6f, 0x0e, "Card transaction error");
    AddCheck(0x6f, 0x0f, "Card write error");
    AddCheck(0x6f, 0x10, "Card counter increment error");
    AddCheck(0x6f, 0x11, "Card counter decrement error");
    AddCheck(0x6f, 0x12, "Card read error");
    AddCheck(0x6f, 0x13, "RC: FIFO overflow");
    AddCheck(0x6f, 0x15, "Framing error in card's answer");
    AddCheck(0x6f, 0x16, "Card access error");
    AddCheck(0x6f, 0x17, "RC: unknown opcode");
    AddCheck(0x6f, 0x18, "A collision has occured");
    AddCheck(0x6f, 0x19, "RC: command execution failed");
    AddCheck(0x6f, 0x1a, "RC: hardware failure");
    AddCheck(0x6f, 0x1b, "RC: timeout");
    AddCheck(0x6f, 0x1c, "Anticollision not supported by the card(s)");
    AddCheck(0x6f, 0x1f, "Bad card status");
    AddCheck(0x6f, 0x20, "Card: vendor specific error");
    AddCheck(0x6f, 0x21, "Card: command not supported");
    AddCheck(0x6f, 0x22, "Card: format of command invalid");
    AddCheck(0x6f, 0x23, "Card: option of command invalid");
    AddCheck(0x6f, 0x24, "other error");
    AddCheck(0x6f, 0x3c, "Reader: invalid parameter");
    AddCheck(0x6f, 0x64, "Reader: invalid opcode");
    AddCheck(0x6f, 0x70, "Reader: internal buffer overflow");
    AddCheck(0x6f, 0x7d, "Reader: invalid length");
}

SpringCardResultChecker::~SpringCardResultChecker()
{
}
}