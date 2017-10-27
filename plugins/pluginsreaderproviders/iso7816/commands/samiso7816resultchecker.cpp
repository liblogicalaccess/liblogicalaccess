/**
 * \file samiso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief SAM ISO7816 result checker.
 */

#include "samiso7816resultchecker.hpp"

namespace logicalaccess
{
SAMISO7816ResultChecker::SAMISO7816ResultChecker()
{
    AddCheck(0x64, 0x00,
             "Memory Range, EEProm busy or access collision in EEPROM module");
    AddCheck(0x64, 0x01, "MFRC52X interface error");

    AddCheck(0x65, 0x81, "High voltage error in SAM EEprom module");

    AddCheck(0x65, 0x01, "Creation of key entry failed");
    AddCheck(0x65, 0x02, "Invalid key reference number");
    AddCheck(0x65, 0x03, "Invalid key usage counter reference number");
    AddCheck(0x69, 0x84, "Key integrity error, wrong key reference");
    AddCheck(0x6A, 0x82, "Invalid key version");

    AddCheck(0x68, 0x83, "Invalid chaining sequence");
    AddCheck(0x69, 0x85, "conditions of use not satisfied, invalid key type, invalid "
                         "CID, key limit reached",
             CardException::WRONG_P1_P2);
    AddCheck(0x69, 0x82, "Integrity error");

    AddCheck(0x90, 0x1E,
             "MAC verification failed, CRC/Padding failed, auth error, integrity error");
}

SAMISO7816ResultChecker::~SAMISO7816ResultChecker()
{
}
}