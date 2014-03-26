/**
 * \file iso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief ISO7816 result checker.
 */

#include "ISO7816ResultChecker.hpp"

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

		AddCheck(0x91, 0x0E, "Insufficient NV-Memory to complete command");
		AddCheck(0x91, 0x1C, "Command code not supported");
		AddCheck(0x91, 0x1E, "CRC or MAC does not match data / Padding bytes not valid");
		AddCheck(0x91, 0x40, "Invalid key number specified");
		AddCheck(0x91, 0x7E, "Length of command string invalid");
		AddCheck(0x91, 0x9D, "Current configuration / status does not allow the requested command");
		AddCheck(0x91, 0x9E, "Value of the parameter(s) invalid");
		AddCheck(0x91, 0xA0, "Requested AID not present on PICC");
		AddCheck(0x91, 0xAE, "Current authentication status does not allow the requested command");
		AddCheck(0x91, 0xBE, "Attempt to read/write data from/to beyond the file's/record's limits");
		AddCheck(0x91, 0xC1, "Unrecoverable error within PICC, PICC will be disabled");
		AddCheck(0x91, 0xCD, "PICC was disabled by an unrecoverable error");
		AddCheck(0x91, 0xCE, "Number of Applications limit reached, no additional CreateApplication possible");
		AddCheck(0x91, 0xDE, "Creation of file/application failed because file/application with same number already exists");
		AddCheck(0x91, 0xEE, "Could not complete NV-write operation due to loss of power, internal backup/rollback mechanism activated");
		AddCheck(0x91, 0xF0, "Specified file number does not exist");
		AddCheck(0x91, 0xF1, "Unrecoverable error within file, file will be disabled");
	}

	ISO7816ResultChecker::~ISO7816ResultChecker() {}
}
