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

		AddCheck(0x67, 0x00, "Wrong length");

		AddCheck(0x68, 0x00, "Invalid CLA byte");

		AddCheck(0x69, 0x81, "Command incompatible");
		AddCheck(0x69, 0x82, "Security status not satisfied");
		AddCheck(0x69, 0x83, "Authentication cannot be done");
		AddCheck(0x69, 0x84, "Reference key not useable");
		AddCheck(0x69, 0x86, "Command not allowed");
		AddCheck(0x69, 0x88, "Key number not valid");

		AddCheck(0x6A, 0x81, "Function not supported");
		AddCheck(0x6A, 0x82, "File not found or addressed block/byte does not exist");

		AddCheck(0x6B, 0x00, "Wrong parameter P1 or P2");
	}

	ISO7816ResultChecker::~ISO7816ResultChecker() {}
}
