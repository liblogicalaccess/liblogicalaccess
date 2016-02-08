#include "stidprgresultchecker.hpp"

using namespace logicalaccess;

STidPRGResultChecker::STidPRGResultChecker()
{
    using ET = CardException::ErrorType;
    AddCheck(0x6D, 0x00, "Incorrect CMD", ET::WRONG_INSTRUCTION);
    AddCheck(0x6B, 0x00, "Invalid P1/P2", ET::WRONG_P1_P2);
    AddCheck(0x61, 0xFF, "No chip present in the RF field");
    AddCheck(0x61, 0xFE, "Wrong password for access to protected blocks");
    AddCheck(0x61, 0xFD, "Incomplete frame sent by the host");
    AddCheck(0x61, 0xFC, "Write operation in the chip failed");
    AddCheck(0x61, 0xFB, "The serial connection is saturated");
    AddCheck(0x61, 0xFA, "The last read command has failed");
    AddCheck(0x61, 0xF9, "The LRC's frame was incorrect, the command failed");
    AddCheck(0x61, 0xF8, "The 'stop_modulation' command for the W03 has failed");
}

bool STidPRGResultChecker::AllowEmptyResult() const
{
    return true;
}
