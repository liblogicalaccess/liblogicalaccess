#include "atrparser.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/logs.hpp"
#include <algorithm>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
ATRParser::ATRParser(const std::vector<uint8_t> &atr)
    : atr_(atr)
{
    // Populated the hardcoded atr table
    register_hardcoded_atr("3B8F8001804F0CA0000003064000000000000028", "Prox",
                           PCSC_RUT_OMNIKEY_XX27);
    register_hardcoded_atr("3B878001C1052F2F0035C730", "MifarePlusS");
    register_hardcoded_atr("3B878001C1052F2F01BCD6A9", "MifarePlusX");
    register_hardcoded_atr("3B8F8001804F0CA000000306030036000000005D",
                           "MifarePlus_SL1_2K");
    register_hardcoded_atr("3B8F8001804F0CA000000306030037000000005C",
                           "MifarePlus_SL1_4K");
    // register_hardcoded_atr("3B8F8001804F0CA0000003060300020000000069",
    //"MifarePlus_SL1_4K");
    register_hardcoded_atr("3B8F8001804F0CA000000306030001000000006A",
                           "MifarePlus_SL1_2K", PCSC_RUT_ACS_ACR_1222L);
    register_hardcoded_atr("3B8F8001804F0CA00000030603FFA00000000034",
                           "MifarePlus_SL1_4K", PCSC_RUT_SPRINGCARD);
    register_hardcoded_atr("3B878001C1052F2F0035C730", "MifarePlus_SL3_2K");
}

///
/// Boilerplate to prepare the proper call to parse()
///

std::string ATRParser::guessCardType(const std::vector<uint8_t> &atr)
{
    ATRParser parser(atr);
    // Type will be ignored.
    return parser.parse(true, PCSCReaderUnitType::PCSC_RUT_ACS_ACR);
}

std::string ATRParser::guessCardType(uint8_t *atr, size_t atrlen)
{
    return guessCardType(std::vector<uint8_t>(atr, atr + atrlen));
}

std::string ATRParser::guessCardType(const std::string &atr_str)
{
    return guessCardType(BufferHelper::fromHexString(atr_str));
}

std::string ATRParser::guessCardType(uint8_t *atr, size_t atrlen,
                                     PCSCReaderUnitType reader_type)
{
    return guessCardType(std::vector<uint8_t>(atr, atr + atrlen), reader_type);
}

std::string ATRParser::guessCardType(const std::string &atr_str,
                                     PCSCReaderUnitType reader_type)
{
    return guessCardType(BufferHelper::fromHexString(atr_str), reader_type);
}

std::string ATRParser::guessCardType(const std::vector<uint8_t> &atr,
                                     PCSCReaderUnitType reader_type)
{
    ATRParser parser(atr);
    return parser.parse(false, reader_type);
}

///
/// ATR Parsing code
///

std::string ATRParser::parse(bool ignore_reader_type,
                             const PCSCReaderUnitType &reader_type) const
{
    LOG(INFOS) << "Trying to match ATR " << atr_ << " ("
               << BufferHelper::getHex(atr_) << ") to a card type.";
    auto atr = check_hardcoded(ignore_reader_type, reader_type);
    if (atr != "UNKNOWN")
        return atr;
    atr = check_from_atr();
    if (atr != "UNKNOWN")
        return atr;
    return check_generic_from_atr();
}

std::string ATRParser::atr_x_to_type(uint8_t code) const
{
    switch (code)
    {
    case 0x01:
        return "Mifare1K";
    case 0x02:
        return "Mifare4K";
    case 0x03:
        return "MifareUltralight";
    case 0x11:
        return "DESFire";
    case 0x1A:
        return "HIDiClass16KS";
    case 0x1C:
        return "HIDiClass8x2KS";
    case 0x18:
        return "HIDiClass2KS";
    case 0x1D:
        return "HIDiClass32KS_16_16";
    case 0x1E:
        return "HIDiClass32KS_16_8x2";
    case 0x1F:
        return "HIDiClass32KS_8x2_16";
    case 0x20:
        return "HIDiClass32KS_8x2_8x2";
    case 0x26:
        return "MIFARE_MINI";
    case 0x3A:
        return "MifareUltralightC";
    }
    return "UNKNOWN";
}

///
/// Hardcoded ATR related code
///

void ATRParser::register_hardcoded_atr(const std::string &atr,
                                       const std::string &card_type)
{
    ATRParser::ATRInfo atr_info;
    atr_info.card_type = card_type;

    hardcoded_atr_to_type_[BufferHelper::fromHexString(atr)] = atr_info;
}

void ATRParser::register_hardcoded_atr(const std::string &atr,
                                       const std::string &card_type,
                                       PCSCReaderUnitType reader_type)
{
    ATRParser::ATRInfo atr_info;
    atr_info.card_type = card_type;
    atr_info.reader_type.push_back(reader_type);

    hardcoded_atr_to_type_[BufferHelper::fromHexString(atr)] = atr_info;
}

std::string ATRParser::check_hardcoded(bool ignore_reader_type,
                                       const PCSCReaderUnitType &reader_type) const
{
    for (const auto &atr_info : hardcoded_atr_to_type_)
    {
        if (atr_info.first == atr_)
        {
            const auto &readers = atr_info.second.reader_type;

            // We only want entry that target all readers (meaning readers.empty() ==
            // true)
            if (ignore_reader_type && readers.empty())
                return atr_info.second.card_type;
            // Try to match the reader type.
            if (readers.empty() ||
                std::find(readers.begin(), readers.end(), reader_type) !=
                    readers.end())
            {
                return atr_info.second.card_type;
            }
        }
    }
    return "UNKNOWN";
}

std::string ATRParser::check_from_atr() const
{
    size_t atrlen      = atr_.size();
    const uint8_t *atr = &atr_[0];

    if (atr && (atrlen > 0))
    {
        unsigned char eatr[20] = {0x3B, 0x8F, 0x80, 0x01, 0x80, 0x4F, 0x0C,
                                  0xA0, 0x00, 0x00, 0x03, 0x06, 0xFF, 0x00,
                                  0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF};

        if (atrlen == 17)
        {
            if ((atr[1] == 0x0F) && (atr[2] == 0xFF))
            {
                switch (atr[16])
                {
                case 0x11:
                    return "Mifare1K";
                case 0x21:
                    return "Mifare4K";
                case 0x93:
                    return "HIDiClass2KS";
                case 0xA3:
                    return "HIDiClass16KS";
                case 0xB3:
                    return "HIDiClass8x2KS";
                }
            }
            else if ((atr[0] == 0x3B) && (atr[1] == 0xF5)) // Specific Mifare classic
                                                           // stuff (coming from
                                                           // smartcard_list)
            {
                unsigned char atrMifare[] = {0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91,
                                             0x81, 0x71, 0xFE, 0x40, 0x00, 0x41,
                                             0x00, 0x00, 0x00, 0x00, 0x05};
                unsigned char atrMifare2[] = {0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91,
                                              0x81, 0x71, 0xFE, 0x40, 0x00, 0x41,
                                              0x08, 0x00, 0x00, 0x00, 0x0D};
                unsigned char atrMifare3[] = {0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91,
                                              0x81, 0x71, 0xFE, 0x40, 0x00, 0x41,
                                              0x18, 0x00, 0x00, 0x00, 0x1D};
                unsigned char atrMifare4[] = {0x3B, 0xF5, 0x91, 0x00, 0xFF, 0x91,
                                              0x81, 0x71, 0xFE, 0x40, 0x00, 0x41,
                                              0x88, 0x00, 0x00, 0x00, 0x8D};

                if (!memcmp(atr, atrMifare, sizeof(atrMifare)))
                {
                    // Contactless Mifare Ultralight
                    return "MifareUltralight";
                }
                else if (!memcmp(atr, atrMifare2, sizeof(atrMifare2)))
                {
                    // Contactless Mifare (Type unknown)
                    return "Mifare1K";
                }
                else if (!memcmp(atr, atrMifare3, sizeof(atrMifare3)))
                {
                    // Contactless Mifare 4k
                    return "Mifare4K";
                }
                else if (!memcmp(atr, atrMifare4, sizeof(atrMifare4)))
                {
                    // Contactless Mifare 1k or 4k
                    return "Mifare1K";
                }
            }
            else
            {
                unsigned char atrFeliCa[] = {0x3B, 0x8C, 0x80, 0x01,
                                             0x04, 0x43, 0xFD};

                if (!memcmp(atr, atrFeliCa, sizeof(atrFeliCa)))
                {
                    return "FeliCa";
                }
            }
            return "UNKNOWN";
        }
        else if (atrlen == 11 && (atr[0] == 0x3B) &&
                 (atr[1] == 0x09)) // specific Mifare classic stuff again (coming
                                   // from smartcard_list)
        {
            unsigned char atrMifare[] = {0x3B, 0x09, 0x41, 0x04, 0x11, 0xDD,
                                         0x82, 0x2F, 0x00, 0x00, 0x88};
            if (!memcmp(atr, atrMifare, sizeof(atrMifare)))
            {
                // 1k contactless Mifare
                return "Mifare1K";
            }
            return "UNKNOWN";
        }
        else if (atrlen == 28 || atrlen == 27)
        {
            // 3B DF 18 FF 81 F1 FE 43 00 3F 03 83 4D 49 46 41 52 45 20 50 6C 75 73
            // 20 53 41 4D 3B NXP SAM AV2 module
            // 3B DF 18 FF 81 F1 FE 43 00 1F 03 4D 49 46 41 52 45 20 50 6C 75 73 20
            // 53 41 4D 98 Mifare SAM AV2
            unsigned char atrTagITP1[] = {0x3B, 0xDF, 0x18, 0xFF, 0x81, 0xF1, 0xFE,
                                          0x43, 0x00, 0x3F, 0x03, 0x83, 0x4D, 0x49,
                                          0x46, 0x41, 0x52, 0x45, 0x20, 0x50, 0x6C,
                                          0x75, 0x73, 0x20, 0x53, 0x41, 0x4D, 0x3B};
            unsigned char atrTagITP2[] = {0x3B, 0xDF, 0x18, 0xFF, 0x81, 0xF1, 0xFE,
                                          0x43, 0x00, 0x1F, 0x03, 0x4D, 0x49, 0x46,
                                          0x41, 0x52, 0x45, 0x20, 0x50, 0x6C, 0x75,
                                          0x73, 0x20, 0x53, 0x41, 0x4D, 0x98};

            if (atrlen == 28 && !memcmp(atr, atrTagITP1, sizeof(atrTagITP1)))
            {
                return "SAM_AV2";
            }
            else if (atrlen == 27 && !memcmp(atr, atrTagITP2, sizeof(atrTagITP2)))
            {
                return "SAM_AV2";
            }
        }
        else
        {
            if (atrlen == 20)
            {
                unsigned char atrCPS3[] = {0x3b, 0x8f, 0x80, 0x01, 0x00, 0x31, 0xb8,
                                           0x64, 0x04, 0xb0, 0xec, 0xc1, 0x73, 0x94,
                                           0x01, 0x80, 0x82, 0x90, 0x00, 0x0e};
                if (!memcmp(atr, atrCPS3, sizeof(atrCPS3)))
                {
                    return "CPS3";
                }
                else
                {
                    memcpy(eatr + 12, atr + 12, 3);
                    eatr[19] = atr[19];

                    if (memcmp(eatr, atr, atrlen) != 0)
                    {
                        return "UNKNOWN";
                    }
                    else
                    {
                        std::string ret = atr_x_to_type((atr[13] << 8) | atr[14]);
                        if (!ret.empty())
                        {
                            return ret;
                        }

                        unsigned char atrTagIT[] = {0x3b, 0x8f, 0x80, 0x01, 0x80,
                                                    0x4f, 0x0c, 0xa0, 0x00, 0x00,
                                                    0x03, 0x06, 0x0b, 0x00, 0x12,
                                                    0x00, 0x00, 0x00, 0x00, 0x71};
                        if (!memcmp(atr, atrTagIT, sizeof(atrTagIT)))
                        {
                            return "TagIt";
                        }
                    }
                }
            }
            else
            {
                if (atrlen == 6)
                {
                    unsigned char atrDESFire[] = {0x3b, 0x81, 0x80,
                                                  0x01, 0x80, 0x80};
                    if (!memcmp(atr, atrDESFire, sizeof(atrDESFire)))
                    {
                        return "DESFire";
                    }
                }
                else if (atrlen == 11)
                {
                    unsigned char atrDESFire[] = {0x3b, 0x86, 0x80, 0x01, 0x06, 0x75,
                                                  0x77, 0x81, 0x02, 0x80, 0x00};
                    if (!memcmp(atr, atrDESFire, sizeof(atrDESFire)))
                    {
                        return "DESFire";
                    }
                }
                else if (atrlen == 19)
                {
                    unsigned char atrDESFire[] = {
                        0x3b, 0xf7, 0x91, 0x00, 0xff, 0x91, 0x81, 0x71, 0xFE, 0x40,
                        0x00, 0x41, 0x20, 0x00, 0x11, 0x77, 0x81, 0x80, 0x40};
                    if (!memcmp(atr, atrDESFire, sizeof(atrDESFire)))
                    {
                        return "DESFire";
                    }
                }
                return "UNKNOWN";
            }
        }
    }
    return "UNKNOWN";
}

std::string ATRParser::check_generic_from_atr() const
{
    size_t atrlen      = atr_.size();
    const uint8_t *atr = &atr_[0];

    EXCEPTION_ASSERT_WITH_LOG(
        atrlen >= 2, LibLogicalAccessException,
        "The ATR length must be at least 2 bytes long (TS + T0).");
    std::string cardType = "UNKNOWN";

    // Check ATR according to PC/SC part3
    unsigned char y  = 0;
    unsigned char TS = atr[y++];
    if (TS != 0x3B) // Direct convention
        return cardType;

    unsigned char T0                    = atr[y++];
    unsigned char historicalBytesLength = T0 & 0x0f;
    bool hasTA1                         = ((T0 & 0x10) == 0x10);
    if (hasTA1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TA1.");
        /*unsigned char TA1 = atr[y++]; */
        y++;
        // Analyze TA1 here
    }
    bool hasTB1 = ((T0 & 0x20) == 0x20);
    if (hasTB1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TB1.");
        /*unsigned char TB1 = atr[y++];*/
        y++;
        // Analyze TB1 here
    }
    bool hasTC1 = ((T0 & 0x40) == 0x40);
    if (hasTC1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TC1.");
        /*unsigned char TC1 = atr[y++]; */
        y++;
        // Analyze TC1 here
    }
    bool hasTD1 = ((T0 & 0x80) == 0x80);

    if (hasTD1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TD1.");
        unsigned char TD1 = atr[y++];
        // isTEqual0Supported = (TD1 & 0x0f) == 0;
        bool hasTA2 = ((TD1 & 0x10) == 0x10);
        if (hasTA2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TA2.");
            /*unsigned char TA2 = atr[y++]; */
            y++;
            // Analyze TA2 here
        }
        bool hasTB2 = ((TD1 & 0x20) == 0x20);
        if (hasTB2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TB2.");
            /*unsigned char TB2 = atr[y++]; */
            y++;
            // Analyze TB2 here
        }
        bool hasTC2 = ((TD1 & 0x40) == 0x40);
        if (hasTC2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TC2.");
            /*unsigned char TC2 = atr[y++];*/
            y++;
            // Analyze TC2 here
        }
        bool hasTD2 = ((TD1 & 0x80) == 0x80);

        if (hasTD2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TD2.");
            unsigned char TD2 = atr[y++];
            // isTEqual1Supported = (TD2 & 0x0f) == 1;
            bool hasTA3 = ((TD2 & 0x10) == 0x10);
            if (hasTA3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TA3.");
                /*unsigned char TA3 = atr[y++];*/
                y++;
                // Analyze TA3 here
            }
            bool hasTB3 = ((TD2 & 0x20) == 0x20);
            if (hasTB3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TB3.");
                /*unsigned char TB3 = atr[y++];*/
                y++;
                // Analyze TB3 here
            }
            bool hasTC3 = ((TD2 & 0x40) == 0x40);
            if (hasTC3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TC3.");
                /*unsigned char TC3 = atr[y++];*/
                y++;
                // Analyze TC3 here
            }
            bool hasTD3 = ((TD2 & 0x80) == 0x80);

            if (hasTD3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TD3.");
                /*unsigned char TD3 = atr[y++];*/
                y++;
                // Analyze TD3 here
            }
        }
    }

    // Historical bytes
    if (historicalBytesLength > 0)
    {
        bool isStorageCard = false;

        try
        {
            unsigned int x = 0;
            EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength,
                                      LibLogicalAccessException,
                                      "Bad historical buffer. Too short to "
                                      "retrieve Category Indicator.");
            /*unsigned char categoryIndicator = atr[y + x++]; // See ISO7816-4,
             * 8.2*/
            x++;

            EXCEPTION_ASSERT_WITH_LOG(
                x < historicalBytesLength, LibLogicalAccessException,
                "Bad historical buffer. Too short to retrieve Application "
                "Identifier Presence Indicator.");
            unsigned char aidIndicator = atr[y + x++];

            EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength,
                                      LibLogicalAccessException,
                                      "Bad historical buffer. Too short to "
                                      "retrieve length from historical bytes.");
            unsigned char length = atr[y + x++];

            if (aidIndicator == 0x4F)
            {
                unsigned int w = 0;
                EXCEPTION_ASSERT_WITH_LOG(length >= 5, LibLogicalAccessException,
                                          "Bad internal historical buffer. Too "
                                          "short to retrieve the RID.");
                unsigned char pcscRID[5] = {0xA0, 0x00, 0x00, 0x03, 0x06};
                unsigned char rid[5];
                memcpy(rid, &atr[y + x + w], sizeof(rid));
                w += sizeof(rid);

                if (memcmp(pcscRID, rid, sizeof(rid)) == 0)
                {
                    EXCEPTION_ASSERT_WITH_LOG(w < length, LibLogicalAccessException,
                                              "Bad internal historical buffer. "
                                              "Too short to retrieve the SS.");
                    unsigned char SS = atr[y + x + w++];

                    switch (SS)
                    {
                    case 0x09:
                    case 0x0a:
                    case 0x0b:
                    case 0x0c:
                        cardType = "ISO15693";
                        break;
                    }

                    EXCEPTION_ASSERT_WITH_LOG(
                        w + 1 < length, LibLogicalAccessException,
                        "Bad internal historical buffer. Too short to retrieve "
                        "the Card Name.");
                    unsigned char cardName[2];
                    memcpy(cardName, &atr[y + x + w], sizeof(cardName));

                    isStorageCard = true;
                }
            }
        }
        catch (LibLogicalAccessException &)
        {
            isStorageCard = false;
        }

        // The format doesn't match a storage card. It should be a CPU card
        if (!isStorageCard)
        {
            unsigned int x = 0;

            // ISO14443-4 Type B
            // It should be the ATQB without checksum (see ISO14443-3, 7.9 for
            // more information on the format)
            if (historicalBytesLength == 12 && atr[y + x] == 0x50)
            {
                cardType = "GENERIC_T_CL_B";
            }
            else
            {
                unsigned int x = 0;
                EXCEPTION_ASSERT_WITH_LOG(x < historicalBytesLength,
                                          LibLogicalAccessException,
                                          "Bad historical buffer. Too short to "
                                          "retrieve Category Indicator.");
                /*unsigned char categoryIndicator = atr[y + x++];	// See
                 * ISO7816-4, 8.2*/
                x++;

                cardType = "GENERIC_T_CL";
            }
        }

        y += historicalBytesLength;
    }

    /* TODO: Disabled now because contact smart cards doesn't have a checksum.
     * Need to fix that ! */

    // Check ATR checksum
    /*
    EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException, "Bad
    "buffer. Too short to retrieve Checksum.");
    unsigned char calcchecksum = 0x00;
    for (unsigned int i = 1; i < y; ++i)
    {
    calcchecksum ^= atr[i];
    }
    unsigned char checksum = atr[y++];
    EXCEPTION_ASSERT_WITH_LOG(checksum == calcchecksum,
    LibLogicalAccessException, "Bad buffer. The checksum doesn't match.");
     */
    return cardType;
}
}
