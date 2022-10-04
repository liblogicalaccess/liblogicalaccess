/**
 * \file twiciso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Twic ISO7816 commands.
 */

#include <logicalaccess/plugins/readers/iso7816/commands/twiciso7816commands.hpp>
#include <logicalaccess/services/accesscontrol/formats/fascn200bitformat.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <logicalaccess/services/accesscontrol/formats/bithelper.hpp>

namespace logicalaccess
{
TwicISO7816Commands::TwicISO7816Commands()
    : ISO7816ISO7816Commands(CMD_TWICISO7816)
{
}

TwicISO7816Commands::TwicISO7816Commands(std::string ct)
    : ISO7816ISO7816Commands(ct)
{
}

TwicISO7816Commands::~TwicISO7816Commands()
{
}

void TwicISO7816Commands::selectTWICApplication()
{
    unsigned char rid[5];
    rid[0] = 0xA0;
    rid[1] = 0x00;
    rid[2] = 0x00;
    rid[3] = 0x03;
    rid[4] = 0x67;

    unsigned char pix[6];
    pix[0] = 0x20;
    pix[1] = 0x00;
    pix[2] = 0x00;
    pix[3] = 0x01;
    pix[4] = 0x81;
    pix[5] = 0x01;

    ByteVector dfname;
    dfname.insert(dfname.begin(), rid, rid + sizeof(rid));
    dfname.insert(dfname.end(), pix, pix + 4);

	auto isocmd = getISO7816Commands();
    try
    {
        // Non gouvernemental card
        isocmd->selectFile(dfname);
    }
    catch (std::exception &)
    {
        // Gouvernemental card
        dfname[sizeof(rid)] = 0x10;
        isocmd->selectFile(dfname);
    }
}

ByteVector TwicISO7816Commands::getUnsignedCardholderUniqueIdentifier()
{
    // return getISO7816Commands()->getData(0x3002);
    return getISO7816Commands()->getDataList(0x5FC104, 0);
}

ByteVector TwicISO7816Commands::getTWICPrivacyKey()
{
    // return getISO7816Commands()->getData(0x2001);*
    return getISO7816Commands()->getDataList(0xDFC101, 0);
}

ByteVector TwicISO7816Commands::getCardholderUniqueIdentifier()
{
    // return getISO7816Commands()->getData(0x3000);
    return getISO7816Commands()->getDataList(0x5FC102, 0);
}

ByteVector TwicISO7816Commands::getCardHolderFingerprints()
{
    // return getISO7816Commands()->getData(0x2003);
    return getISO7816Commands()->getDataList(0xDFC103, 0);
}

ByteVector TwicISO7816Commands::getSecurityObject()
{
    // return getISO7816Commands()->getData(0x9000);
    return getISO7816Commands()->getDataList(0xDFC10F, 0);
}
}