/**
 * \file samiso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief SAM ISO7816 result checker.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/samiso7816resultchecker.hpp>

namespace logicalaccess
{
SAMISO7816ResultChecker::SAMISO7816ResultChecker()
{
    AddCheck(0x64, 0x00, "Memory Range, EEPROM busy or access collision in EEPROM module");
    AddCheck(0x64, 0x01, "MFRC52X interface error");
    AddCheck(0x65, 0x01, "Creation of key entry failed");
    AddCheck(0x65, 0x02, "Invalid key reference number");
    AddCheck(0x65, 0x03, "Invalid key usage counter reference number");
    AddCheck(0x65, 0x81, "High voltage error in SAM EEPROM module");
    AddCheck(0x67, 0x00, "APDU rejected: invalid Lc or wrong APDU length",
             CardException::WRONG_LENGTH);
    AddCheck(0x68, 0x83, "Invalid chaining sequence");
    AddCheck(0x69, 0x82, "Integrity error");
    AddCheck(0x69, 0x84, "Key integrity error, wrong key reference");
    AddCheck(0x69, 0x85,
             "Conditions of use not satisfied, invalid key type, invalid "
             "CID, or key limit reached",
             CardException::SECURITY_STATUS);
    AddCheck(0x6A, 0x82, "Invalid key version");
    AddCheck(0x6A, 0x84, "SAM Host protection error", CardException::SECURITY_STATUS);
    AddCheck(0x6A, 0x86, "Incorrect APDU framing (wrong P1 or P2)");
    AddCheck(0x6C, 0x00, "Invalid Le", CardException::WRONG_LE);
    AddCheck(0x90, 0x1E, "MAC verification failed, CRC/Padding failed, authentication error, or integrity error");
}

SAMISO7816ResultChecker::~SAMISO7816ResultChecker()
{
}
}