/**
 * \file desfireiso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief DESFire ISO 7816 result checker.
 */

#include "mifareomnikeyxx27resultchecker.hpp"

namespace logicalaccess
{
    MifareOmnikeyXX27ResultChecker::MifareOmnikeyXX27ResultChecker()
    {
        AddCheck(0x62, 0x82, "End of data reached before Le bytes (Le is greater than data length).");

        AddCheck(0x64, 0x00, "No Response from media (Time Out)");

        AddCheck(0x65, 0x81, "Illegal block number (out of memory space)");

        AddCheck(0x67, 0x00, "Wrong length (Lc or Le)");

        AddCheck(0x69, 0x81, "Incompatible command");
        AddCheck(0x69, 0x82, "Security status not satisfied (not authenticated)");
        AddCheck(0x69, 0x84, "KeyStructure incorrect");
        AddCheck(0x69, 0x86, "Wrong key type");
        AddCheck(0x69, 0x88, "KeyNumber not valid");
        AddCheck(0x69, 0x89, "KeyLength incorrect");

        AddCheck(0x6A, 0x81, "Function not supported");
        AddCheck(0x6A, 0x82, "Illegal block number (File not found)");

        AddCheck(0x6B, 0x00, "Wrong parameter (P1 or P2)");
    }

    MifareOmnikeyXX27ResultChecker::~MifareOmnikeyXX27ResultChecker() {}
}