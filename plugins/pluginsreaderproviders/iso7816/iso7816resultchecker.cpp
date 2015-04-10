/**
 * \file iso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief ISO7816 result checker.
 */

#include "iso7816resultchecker.hpp"

namespace logicalaccess
{
    ISO7816ResultChecker::ISO7816ResultChecker()
    {
        AddCheck(0x64, 0x00, "Execution error");

        AddCheck(0x65, 0x81, "Memory failure");

        AddCheck(0x67, 0x00, "Wrong Length or LC byte");

        AddCheck(0x68, 0x00, "Invalid CLA byte");

        AddCheck(0x69, 0x81, "Command incompatible");
        AddCheck(0x69, 0x82, "Security status not satisfied");
        AddCheck(0x69, 0x83, "Authentication cannot be done");
        AddCheck(0x69, 0x84, "Reference key not useable");
        AddCheck(0x69, 0x86, "Command not allowed");
        AddCheck(0x69, 0x88, "Key number not valid");

        AddCheck(0x6A, 0x81, "Function not supported");
        AddCheck(0x6A, 0x82, "File not found or addressed block/byte does not exist");
        AddCheck(0x6A, 0x86, "Wrong P1 P2");

        AddCheck(0x6B, 0x00, "Wrong parameter P1 or P2");

        AddCheck(0x6C, 0x00, "Wrong LE");

        AddCheck(0x6D, 0x00, "Wrong Instruction");

        AddCheck(0x6E, 0x00, "Wrong Class");

        AddCheck(0x6F, 0x00, "Operation failed, no precise diagnosis");

        AddCheck(0x69, 0x86, "Command not allowed");

        AddCheck(0x6A, 0x80, "Wrong parameters for current instruction");
    }

    ISO7816ResultChecker::~ISO7816ResultChecker() {}
}