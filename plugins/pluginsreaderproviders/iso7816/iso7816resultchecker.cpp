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
        AddCheck(0x64, 0x00, "Execution error", CardException::EXECUTION_ERROR);

        AddCheck(0x65, 0x81, "Memory failure", CardException::MEMORY_FAILURE);

        AddCheck(0x67, 0x00, "Wrong Length or LC byte", CardException::WRONG_LENGTH);

        AddCheck(0x68, 0x00, "Invalid CLA byte", CardException::INVALID_CLA_BYTE);

        AddCheck(0x69, 0x81, "Command incompatible", CardException::COMMAND_INCOMPATIBLE);
        AddCheck(0x69, 0x82, "Security status not satisfied", CardException::SECURITY_STATUS);
        AddCheck(0x69, 0x83, "Authentication cannot be done", CardException::AUTHENTICATION_FAILURE);
        AddCheck(0x69, 0x84, "Reference key not useable", CardException::REFERENCE_KEY_UNUSABLE);
        AddCheck(0x69, 0x86, "Command not allowed", CardException::COMMAND_NOT_ALLOWED);
        AddCheck(0x69, 0x88, "Key number not valid", CardException::KEY_NUMBER_INVALID);

        AddCheck(0x6A, 0x81, "Function not supported", CardException::FUNCTION_NOT_SUPPORTED);
        AddCheck(0x6A, 0x82, "File not found or addressed block/byte does not exist",
			CardException::NOT_FOUND);
        AddCheck(0x6A, 0x86, "Wrong P1 P2", CardException::WRONG_P1_P2);

        AddCheck(0x6B, 0x00, "Wrong parameter P1 or P2", CardException::WRONG_P1_P2);

        AddCheck(0x6C, 0x00, "Wrong LE", CardException::WRONG_LE);

        AddCheck(0x6D, 0x00, "Wrong Instruction", CardException::WRONG_INSTRUCTION);

        AddCheck(0x6E, 0x00, "Wrong Class", CardException::WRONG_CLASS);

        AddCheck(0x6F, 0x00, "Operation failed, no precise diagnosis", CardException::UNKOWN_ERROR);

        AddCheck(0x6A, 0x80, "Wrong parameters for current instruction", CardException::WRONG_P1_P2);
    }

    ISO7816ResultChecker::~ISO7816ResultChecker() {}
}