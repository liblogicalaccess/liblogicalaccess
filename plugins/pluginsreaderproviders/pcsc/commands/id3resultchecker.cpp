#include "id3resultchecker.hpp"

using namespace logicalaccess;

ID3ResultChecker::ID3ResultChecker()
{
    AddCheck(0x63, 0x00,
             "PIN is rejected by reader");

    AddCheck(0x64, 0x00, "Command execution error");
    AddCheck(0x67, 0x00, "Command and its parameters have a wrong length");
    AddCheck(0x69, 0x82, "Security Conditions are not satisfied; Authentication is not performed, "
        "PIN is not verified, etc...");
    AddCheck(0x69, 0x83, "Cryptographic key type is not supported.");
    AddCheck(0x69, 0x84, "Specified key is locked.");
    AddCheck(0x69, 0x85, "Secured transmissions are not supported.");
    AddCheck(0x69, 0x86, "Volatile memory is not supported.");
    AddCheck(0x69, 0x87, "A secure messaging data object is missing.");
    AddCheck(0x69, 0x88, "Key number is invalid or does not exist.");
    AddCheck(0x69, 0x89, "Bad key length.");
    AddCheck(0x6A, 0x00, "P2 byte has an invalid value.");
    AddCheck(0x6A, 0x80, "One or more command parameter is invalid.");
    AddCheck(0x6A, 0x81, "Command or Function not supported.");
    AddCheck(0x6B, 0x00, "P1 and P2 bytes have an invalid value.");
    AddCheck(0x6D, 0x00, "Instruction Byte is invalid or not supported.");
    AddCheck(0x6F, 0x00, "Error occurs but reader gives no error details.");
    AddCheck(0x6F, 0xFD, "Card type is not suitable for this operation.");
    AddCheck(0x6F, 0xFE, "Current selected card is invalid.");
    AddCheck(0x6F, 0xFF, "Card has been removed or does not answer.");
}
