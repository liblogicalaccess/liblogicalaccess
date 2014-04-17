/**
 * \file desfireiso7816resultchecker.cpp
 * \author Arnaud H. <arnaud-dev@islog.com>
 * \brief DESFire ISO 7816 result checker.
 */

#include "desfireiso7816resultchecker.hpp"

namespace logicalaccess
{
	DESFireISO7816ResultChecker::DESFireISO7816ResultChecker()
	{
		AddCheck(0x62, 0x82, "End of file reached before reading Le bytes");
		
		AddCheck(0x65, 0x81, "Memory failure (unsuccessful updating)");

		AddCheck(0x69, 0x82, "File access not allowed");
		AddCheck(0x69, 0x85, "File empty / Access conditions not satisfied");
		
		AddCheck(0x6A, 0x82, "File not found");
		AddCheck(0x6A, 0x86, "Wrong parameter P1 and/or P2");
		AddCheck(0x6A, 0x87, "Wrong parameter Lc inconsistent with P1-P2");
		
		AddCheck(0x6C, 0x00, "File not found");
		
		AddCheck(0x6F, 0x00, "No precise diagnostics");

		AddCheck(0x91, 0x0C, "No changes done to backup files. CommitTransaction / AbortTransaction not necessary", false);
		AddCheck(0x91, 0x0E, "Insufficient NV-Memory to complete command");
		AddCheck(0x91, 0x1C, "Command code not supported");
		AddCheck(0x91, 0x1E, "CRC or MAC does not match data / Padding bytes not valid");
		AddCheck(0x91, 0x40, "Invalid key number specified");
		AddCheck(0x91, 0x7E, "Length of command string invalid");
		AddCheck(0x91, 0x9D, "Current configuration / status does not allow the requested command");
		AddCheck(0x91, 0x9E, "Value of the parameter(s) invalid");
		AddCheck(0x91, 0xA0, "Requested AID not present on PICC");
		AddCheck(0x91, 0xA1, "Unrecoverable error within aplication, application will be disabled");
		AddCheck(0x91, 0xAE, "Current authentication status does not allow the requested command");
		AddCheck(0x91, 0xAF, "Additional data frame is expected to be sent", false);
		AddCheck(0x91, 0xBE, "Attempt to read/write data from/to beyond the file's/record's limits / Attempt to exceed the limits of a value file");
		AddCheck(0x91, 0xC1, "Unrecoverable error within PICC, PICC will be disabled");
		AddCheck(0x91, 0xCA, "Previous Command was not fully completed / Not all Frames were requested or provided by the PCD", false);
		AddCheck(0x91, 0xCD, "PICC was disabled by an unrecoverable error");
		AddCheck(0x91, 0xCE, "Number of Applications limit reached, no additional CreateApplication possible");
		AddCheck(0x91, 0xDE, "Creation of file/application failed because file/application with same number already exists");
		AddCheck(0x91, 0xEE, "Could not complete NV-write operation due to loss of power, internal backup/rollback mechanism activated");
		AddCheck(0x91, 0xF0, "Specified file number does not exist");
		AddCheck(0x91, 0xF1, "Unrecoverable error within file, file will be disabled");
	}

	DESFireISO7816ResultChecker::~DESFireISO7816ResultChecker() {}
}
