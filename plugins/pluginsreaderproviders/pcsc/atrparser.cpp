#include "atrparser.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "logicalaccess/logs.hpp"
#include <algorithm>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
ATRParser::ATRParser(const ByteVector &atr)
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
    register_hardcoded_atr("3BF59100FF918171FE400041080000000D", "Mifare1K");
    register_hardcoded_atr("3BF59100FF918171FE400041180000001D", "Mifare4K");
    register_hardcoded_atr("3BF59100FF918171FE400041880000008D", "Mifare1K");
    register_hardcoded_atr("3B09410411DD822F000088", "Mifare1K");
    register_hardcoded_atr("3B8F8001804F0CA000000306030000000000006B", "Mifare1K",
                           PCSC_RUT_ID3_CL1356);
    register_hardcoded_atr("3B8180018080", "DESFire");
    register_hardcoded_atr("3B86800106757781028000", "DESFire");
    register_hardcoded_atr("3BF79100FF918171FE40004120001177818040", "DESFire");
    register_hardcoded_atr("3BF59100FF918171FE4000410x0000000005", "MifareUltralight");
    register_hardcoded_atr("3B8C80010443FD", "FeliCa");
    register_hardcoded_atr("3B8F80010031B86404B0ECC1739401808290000E", "CPS3");
    register_hardcoded_atr("3B8F8001804F0CA0000003060B00120000000071", "TagIt");
    register_hardcoded_atr("3B8F8001804F0CA00000030603F004000000009F", "Topaz");
    register_hardcoded_atr("3BDF18FF81F1FE43003F03834D494641524520506C75732053414D3B",
                           "SAM_AV2");
    register_hardcoded_atr("3BDF18FF81F1FE43001F034D494641524520506C75732053414D98",
                           "SAM_AV2");

    // SEOS or Electronic Passport / Spanish passport (2012)
    register_hardcoded_atr("3B80800101", "SEOS");
}

///
/// Boilerplate to prepare the proper call to parse()
///

std::string ATRParser::guessCardType(const ByteVector &atr)
{
    ATRParser parser(atr);
    // Type will be ignored.
    return parser.parse(true, PCSC_RUT_ACS_ACR);
}

std::string ATRParser::guessCardType(uint8_t *atr, size_t atrlen)
{
    return guessCardType(ByteVector(atr, atr + atrlen));
}

std::string ATRParser::guessCardType(const std::string &atr_str)
{
    return guessCardType(BufferHelper::fromHexString(atr_str));
}

std::string ATRParser::guessCardType(uint8_t *atr, size_t atrlen,
                                     PCSCReaderUnitType reader_type)
{
    return guessCardType(ByteVector(atr, atr + atrlen), reader_type);
}

std::string ATRParser::guessCardType(const std::string &atr_str,
                                     PCSCReaderUnitType reader_type)
{
    return guessCardType(BufferHelper::fromHexString(atr_str), reader_type);
}

std::string ATRParser::guessCardType(const ByteVector &atr,
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
    LOG(INFOS) << "Trying to match ATR " << atr_ << " (" << BufferHelper::getHex(atr_)
               << ") to a card type.";
    auto atr = check_hardcoded(ignore_reader_type, reader_type);
    if (atr != "UNKNOWN")
        return atr;
    atr = check_from_atr();
    if (atr != "UNKNOWN")
        return atr;
    return check_generic_from_atr();
}

std::string ATRParser::atr_x_to_type(uint8_t code)
{
    switch (code)
    {
    case 0x01: return "Mifare1K";
    case 0x02: return "Mifare4K";
    case 0x03: return "MifareUltralight";
    case 0x11: return "DESFire";
    case 0x12: return "TagIt";
    case 0x16: return "iCode1";
    case 0x1A: return "HIDiClass16KS";
    case 0x1C: return "HIDiClass8x2KS";
    case 0x18: return "HIDiClass2KS";
    case 0x1D: return "HIDiClass32KS_16_16";
    case 0x1E: return "HIDiClass32KS_16_8x2";
    case 0x1F: return "HIDiClass32KS_8x2_16";
    case 0x20: return "HIDiClass32KS_8x2_8x2";
    case 0x26: return "MifareMini";
    case 0x2F:
        return "Topaz"; // Jewel
    case 0x30: return "Topaz";
    case 0x35: return "iCode2";
    case 0x36: return "MifarePlus_SL1_2K";
    case 0x37: return "MifarePlus_SL1_4K";
    case 0x38:
        return "MifarePlus_SL1_2K"; // Should be MifarePlus_SL2_2K
    case 0x39:
        return "MifarePlus_SL1_4K"; // Should be MifarePlus_SL2_4K
    case 0x3A: return "MifareUltralightC";
    case 0x3B: return "FeliCa";
    default:;
    }
    return "UNKNOWN";
}

///
/// Hardcoded ATR related code
///

void ATRParser::register_hardcoded_atr(const std::string &atr,
                                       const std::string &card_type)
{
    ATRInfo atr_info;
    atr_info.card_type = card_type;

    hardcoded_atr_to_type_[BufferHelper::fromHexString(atr)] = atr_info;
}

void ATRParser::register_hardcoded_atr(const std::string &atr,
                                       const std::string &card_type,
                                       PCSCReaderUnitType reader_type)
{
    ATRInfo atr_info;
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
                find(readers.begin(), readers.end(), reader_type) != readers.end())
            {
                return atr_info.second.card_type;
            }
        }
    }
    return "UNKNOWN";
}

std::string ATRParser::check_from_atr() const
{
    size_t atrlen = atr_.size();
    if (atrlen == 0)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException, "ATR is missing.");
    const uint8_t *atr = &atr_[0];

    if (atr && (atrlen > 0))
    {
        if (atrlen == 7)
        {
            unsigned char atrTopaz[] = {0x3b, 0x82, 0x80,
                                        0x01, 0x02, 0x44}; // Don't check last byte
            if (!memcmp(atr, atrTopaz, sizeof(atrTopaz)))
            {
                return "Topaz";
            }
        }
        else if (atrlen == 17)
        {
            if ((atr[1] == 0x0F) && (atr[2] == 0xFF))
            {
                switch (atr[16])
                {
                case 0x11: return "Mifare1K";
                case 0x21: return "Mifare4K";
                case 0x93: return "HIDiClass2KS";
                case 0xA3: return "HIDiClass16KS";
                case 0xB3: return "HIDiClass8x2KS";
                default:;
                }
            }
        }
        else if (atrlen == 20)
        {
            unsigned char eatr[20] = {0x3B, 0x8F, 0x80, 0x01, 0x80, 0x4F, 0x0C,
                                      0xA0, 0x00, 0x00, 0x03, 0x06, 0xFF, 0x00,
                                      0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF};

            memcpy(eatr + 12, atr + 12, 3);
            eatr[19] = atr[19];

            if (memcmp(eatr, atr, atrlen) == 0)
            {
                std::string ret = atr_x_to_type((atr[13] << 8) | atr[14]);
                if (!ret.empty())
                {
                    return ret;
                }
            }
        }
    }

    return "UNKNOWN";
}

std::string ATRParser::check_generic_from_atr() const
{
    size_t atrlen      = atr_.size();
    const uint8_t *atr = &atr_[0];

    EXCEPTION_ASSERT_WITH_LOG(atrlen >= 2, LibLogicalAccessException,
                              "The ATR length must be at least 2 bytes long (TS + T0).");
    std::string cardType = "UNKNOWN";

    // Check ATR according to PC/SC part3
    unsigned char y  = 0;
    unsigned char TS = atr[y++];
    if (TS != 0x3B) // Direct convention
        return cardType;

    unsigned char T0                    = atr[y++];
    unsigned char historicalBytesLength = T0 & 0x0f;
    const bool hasTA1                   = ((T0 & 0x10) == 0x10);
    if (hasTA1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TA1.");
        /*unsigned char TA1 = atr[y++]; */
        y++;
        // Analyze TA1 here
    }
    const bool hasTB1 = ((T0 & 0x20) == 0x20);
    if (hasTB1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TB1.");
        /*unsigned char TB1 = atr[y++];*/
        y++;
        // Analyze TB1 here
    }
    const bool hasTC1 = ((T0 & 0x40) == 0x40);
    if (hasTC1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TC1.");
        /*unsigned char TC1 = atr[y++]; */
        y++;
        // Analyze TC1 here
    }
    const bool hasTD1 = ((T0 & 0x80) == 0x80);

    if (hasTD1)
    {
        EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                  "Bad buffer. Too short to retrieve TD1.");
        const unsigned char TD1 = atr[y++];
        // isTEqual0Supported = (TD1 & 0x0f) == 0;
        const bool hasTA2 = ((TD1 & 0x10) == 0x10);
        if (hasTA2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TA2.");
            /*unsigned char TA2 = atr[y++]; */
            y++;
            // Analyze TA2 here
        }
        const bool hasTB2 = ((TD1 & 0x20) == 0x20);
        if (hasTB2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TB2.");
            /*unsigned char TB2 = atr[y++]; */
            y++;
            // Analyze TB2 here
        }
        const bool hasTC2 = ((TD1 & 0x40) == 0x40);
        if (hasTC2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TC2.");
            /*unsigned char TC2 = atr[y++];*/
            y++;
            // Analyze TC2 here
        }
        const bool hasTD2 = ((TD1 & 0x80) == 0x80);

        if (hasTD2)
        {
            EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                      "Bad buffer. Too short to retrieve TD2.");
            const unsigned char TD2 = atr[y++];
            // isTEqual1Supported = (TD2 & 0x0f) == 1;
            const bool hasTA3 = ((TD2 & 0x10) == 0x10);
            if (hasTA3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TA3.");
                /*unsigned char TA3 = atr[y++];*/
                y++;
                // Analyze TA3 here
            }
            const bool hasTB3 = ((TD2 & 0x20) == 0x20);
            if (hasTB3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TB3.");
                /*unsigned char TB3 = atr[y++];*/
                y++;
                // Analyze TB3 here
            }
            const bool hasTC3 = ((TD2 & 0x40) == 0x40);
            if (hasTC3)
            {
                EXCEPTION_ASSERT_WITH_LOG(y < atrlen, LibLogicalAccessException,
                                          "Bad buffer. Too short to retrieve TC3.");
                /*unsigned char TC3 = atr[y++];*/
                y++;
                // Analyze TC3 here
            }
            const bool hasTD3 = ((TD2 & 0x80) == 0x80);

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
            const unsigned char aidIndicator = atr[y + x++];

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
                    const unsigned char SS = atr[y + x + w++];

                    switch (SS)
                    {
                    case 0x09:
                    case 0x0a:
                    case 0x0b:
                    case 0x0c: cardType = "ISO15693"; break;
                    default:;
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
