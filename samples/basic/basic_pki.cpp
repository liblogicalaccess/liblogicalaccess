#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav3iso7816commands.hpp>

#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/bn.h>

using namespace logicalaccess;

constexpr bool RUN_NEGATIVE_TESTS     = false;
constexpr bool REAUTH_AFTER_EACH_TEST = true;

namespace pki
{
enum class HashAlgo : unsigned char
{
    SHA1   = 0x00,
    SHA224 = 0x01,
    SHA256 = 0x03,
    RFU    = 0xFF
};

using Config = uint16_t;

// PKI configuration flags
constexpr Config PUBLIC_KEY                    = 0;
constexpr Config PRIVATE_KEY                   = (1u << 0);
constexpr Config ALLOW_PRIVATE_EXPORT          = (1u << 1);
constexpr Config DISABLE                       = (1u << 2);
constexpr Config DISABLE_ENCRYPTION            = (1u << 3);
constexpr Config DISABLE_SIGNATURE             = (1u << 4);
constexpr Config UPDATE_KEY_ENTRIES            = (1u << 5);
constexpr Config CRT                           = (1u << 6);
constexpr Config ENCIPHER_KEYS                 = (1u << 7);
constexpr Config FORCE_HOST_USAGE              = (1u << 8);
constexpr Config FORCE_HOST_CHANGE             = (1u << 9);
// Test defaults
constexpr uint8_t DEFAULT_KUC    = 0xFE;
constexpr uint8_t DEFAULT_CEKNO  = 0x00;
constexpr uint8_t DEFAULT_CEKVER = 0xFF;
constexpr uint8_t FIRST_SLOT     = 0x00;
constexpr uint8_t LAST_SLOT      = 0x02;

const Config TEST_KEY = PRIVATE_KEY | ALLOW_PRIVATE_EXPORT | UPDATE_KEY_ENTRIES | ENCIPHER_KEYS | CRT;

const Config TEST_KEY_SAFE = PRIVATE_KEY | ALLOW_PRIVATE_EXPORT | CRT;
}

struct PKIImportTestCase
{
    unsigned char keyNo{};
    unsigned short config{};
    unsigned char keyNoCEK{};
    unsigned char keyNoVCEK{};
    unsigned char refNoKUC{};
    unsigned char keyNoAEK{};
    unsigned char keyVAEK{};
    bool provideAEK{};
    ByteVector n{};
    ByteVector e{};
    ByteVector p{};
    ByteVector q{};
    ByteVector dP{};
    ByteVector dQ{};
    ByteVector iPQ{};
    bool includeAccess{};
    bool updateSettingsOnly{};
    bool expectSuccess{};
    std::string description;
};

namespace util
{
inline ByteVector makeVector(size_t size, uint8_t value = 0x01)
{
    return ByteVector(size, value);
}

inline ByteVector defaultExponent()
{
    return BufferHelper::fromHexString("00010001");
}

inline const ByteVector &E()
{
    static const ByteVector value = defaultExponent();
    return value;
}

inline const ByteVector &N512()
{
    static const ByteVector value = makeVector(0x40, 0xAA);
    return value;
}

inline const ByteVector &N1024()
{
    static const ByteVector value = makeVector(0x80, 0xBB);
    return value;
}

inline const ByteVector &N2048()
{
    static const ByteVector value = makeVector(0x100, 0xAA);
    return value;
}

inline const ByteVector &P256()
{
    static const ByteVector value = makeVector(0x20, 0x11);
    return value;
}

inline const ByteVector &Q256()
{
    static const ByteVector value = makeVector(0x20, 0x22);
    return value;
}

inline const ByteVector &DP256()
{
    static const ByteVector value = makeVector(0x20, 0x33);
    return value;
}

inline const ByteVector &DQ256()
{
    static const ByteVector value = makeVector(0x20, 0x44);
    return value;
}

inline const ByteVector &IPQ256()
{
    static const ByteVector value = makeVector(0x20, 0x55);
    return value;
}

// PKIImportTestCase factories
inline PKIImportTestCase makePKIImportTest()
{
    PKIImportTestCase tc{};
    tc.keyNo     = 0x01;
    tc.config    = pki::CRT;
    tc.keyNoCEK  = 0xFE;
    tc.keyNoVCEK = 0x00;
    tc.refNoKUC  = 0xFF;
    tc.provideAEK = false;
    tc.keyNoAEK = 0x00;
    tc.keyVAEK = 0x00;
    tc.includeAccess      = false;
    tc.updateSettingsOnly = false;
    tc.expectSuccess      = true;
    return tc;
}

inline PKIImportTestCase makePKIPublicKeyTest()
{
    auto tc   = makePKIImportTest();
    tc.config = pki::PUBLIC_KEY | pki::CRT;
    tc.n      = N1024();
    tc.e      = E();
    return tc;
}

inline PKIImportTestCase makePKICRTKeyTest()
{
    auto tc   = makePKIImportTest();
    tc.config = pki::PRIVATE_KEY | pki::CRT;
    tc.n      = N512();
    tc.e      = E();
    tc.p      = P256();
    tc.q      = Q256();
    tc.dP     = DP256();
    tc.dQ     = DQ256();
    tc.iPQ    = IPQ256();
    return tc;
}
}

struct PKIGenerateTestCase
{
    unsigned char keyNo{};
    unsigned short config{};
    unsigned char keyNoCEK{};
    unsigned char keyNoVCEK{};
    unsigned char keyNoRef{};
    sam::AEKVAEK accessKeys{};
    unsigned short nLen{};
    ByteVector pki_e{};
    bool includeAccess{};
    bool expectSuccess{};
    bool expectDisabled{};
    std::string description;
};

struct PKIExportPrivateTestCase
{
    unsigned char keyNo{};
    bool requestAEK{};
    bool expectSuccess{};
    bool expectAEK{};
    bool expectPrivatePartPresent{};
    std::string description;
};

struct PKIExportPublicTestCase
{
    unsigned char keyNo{};
    bool returnAEK{};
    bool expectSuccess{};
    bool expectAEK{};
    std::string description;
};

struct PKIUpdateKeyEntriesTestCase
{
    unsigned char keyNoEnc{};
    unsigned char keyNoSign{};
    bool requestAck{};
    unsigned char keyNoAck{};
    unsigned char hashAlgo{};
    size_t nbEntries{};
    bool expectSuccess{};
    std::string description;
};

struct PKIGenerateHashTestCase
{
    unsigned char hashAlgo{};
    ByteVector data{};
    bool expectSuccess{};
    std::string description;
};

struct PKIGenerateSignatureTestCase
{
    unsigned char hashAlgo{};
    unsigned char keyNo{};
    ByteVector data{};
    bool expectSuccess{};
    std::string description;
};

struct PKISendSignatureTestCase
{
    bool generateBeforeSend{};
    bool expectSuccess{};
    std::string description;
};

std::vector<PKIExportPrivateTestCase> exportPrivateKeyTests = {
    // ===== SUCCESS =====
    {0x00, false, true, true, true, "(STABLE) Export private key (no AEK)"},
    {0x00, false, true, false, true, "Export CRT private key"},
    {0x01, false, true, true, true, "(STABLE) Export private key (no AEK)"},
    {0x01, false, true, false, true, "Export CRT private key"},

    // ===== NO PRIVATE KEY PRESENT =====
    {0x00, false, false, false, false, "Public key slot exported as private"},

    // ===== PRIVATE EXPORT DISABLED =====
    {0x01, false, false, false, false, "Private export disabled in PKI_SET"},

    // ===== INVALID KEY REFERENCES =====
    {0x02, false, false, false, false, "keyNo 2 not allowed for private export"},
    {0x03, false, false, false, false, "keyNo above range"},
    {0xFF, false, false, false, false, "keyNo overflow"},

    // ===== CHAINING =====
    {0x01, false, true, false, true, "2048-bit CRT export chaining"},
    {0x01, true, true, true, true, "2048-bit CRT export chaining + AEK"},

    // ===== REPEATABILITY =====
    {0x01, false, true, false, true, "Repeated export #1"},
    {0x01, false, true, false, true, "Repeated export #2"},
    {0x01, false, true, false, true, "Repeated export #3"}
};

std::vector<PKIExportPublicTestCase> exportPublicKeyTests = {
    // ===== VALID PUBLIC EXPORTS =====
    {0x00, false, true, false, "(STABLE) Valid export public key slot 0"},
    {0x01, false, true, false, "Valid export public part of private key slot 1"},
    {0x02, false, true, false, "Valid export public key slot 2"},

    // ===== AEK RETURN =====
    {0x00, true, true, true, "Valid export slot 0 with AEK"},
    {0x01, true, true, true, "Valid export slot 1 with AEK"},
    {0x02, true, true, true, "Valid export slot 2 with AEK"},

    // ===== BOUNDARY KEY NUMBERS =====
    {0x03, false, false, false, "Invalid keyNo out of range (0x03)"},
    {0x04, false, false, false, "Invalid keyNo mid-range invalid"},
    {0xFF, false, false, false, "Invalid keyNo overflow"},

    // ===== CHAINING =====
    {0x01, false, true, false, "2048-bit public export chaining"},
    {0x01, false, true, false, "2048-bit public export chaining + AEK"},

    // ===== REPEATABILITY =====
    {0x00, false, true, false, "Repeated export #1"},
    {0x00, false, true, false, "Repeated export #2"},
    {0x00, false, true, false, "Repeated export #3"}
};

std::vector<PKIImportTestCase> importTests = {

    // ===== SUCCESS =====
    []
    {
        auto tc        = util::makePKIPublicKeyTest();
        tc.description = "Valid public key import (1024-bit)";
        return tc;
    }(),

    []
    {
        auto tc        = util::makePKIImportTest();
        tc.keyNo       = 0x02;
        tc.n           = util::N512();
        tc.e           = util::E();
        tc.description = "Valid public key at keyNo = 2";
        return tc;
    }(),

    // ===== SETTINGS =====
    []
    {
        auto tc               = util::makePKIImportTest();
        tc.config             = pki::DISABLE | pki::CRT;
        tc.updateSettingsOnly = true;
        tc.description        = "Settings-only disable key entry";
        return tc;
    }(),

    []
    {
        auto tc               = util::makePKIImportTest();
        tc.updateSettingsOnly = true;
        tc.description        = "Settings-only update without RSA data";
        return tc;
    }(),

    []
    {
        auto tc               = util::makePKIImportTest();
        tc.keyNo              = 0x02;
        tc.config             = pki::DISABLE | pki::CRT;
        tc.updateSettingsOnly = true;
        tc.description        = "Settings-only on keyNo=2";
        return tc;
    }(),

    // ===== INVALID RSA =====
    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::makeVector(0x41, 0xAA);
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Invalid modulus length (not multiple of 8)";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::makeVector(0x38, 0xAA);
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Modulus too small";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::makeVector(0x108, 0xAA);
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Modulus too large";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::makeVector(0x40, 0x00);
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Invalid modulus MSB = 0";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::N512();
        tc.e             = BufferHelper::fromHexString("020002");
        tc.expectSuccess = false;
        tc.description   = "Exponent even";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::N512();
        tc.e             = BufferHelper::fromHexString("010001");
        tc.expectSuccess = false;
        tc.description   = "Exponent too small (3 bytes)";
        return tc;
    }(),

    []
    {
        auto tc        = util::makePKIImportTest();
        tc.n           = util::N512();
        tc.e           = util::E();
        tc.description = "Exponent 4 bytes valid";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Missing modulus";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::N512();
        tc.expectSuccess = false;
        tc.description   = "Missing exponent";
        return tc;
    }(),

    // ===== AEK =====
    /*[]
    {
        auto tc          = util::makePKIImportTest();
        tc.n             = util::N512();
        tc.e             = util::E();
        tc.keyNoAEK      = 0x05;
        tc.keyVAEK       = 0x01;
        tc.provideAEK    = true;
        tc.includeAccess = true;
        tc.description   = "Valid AEK restriction";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.config        = pki::DISABLE | pki::CRT;
        tc.n             = util::N512();
        tc.e             = util::E();
        tc.keyNoAEK      = 0x05;
        tc.keyVAEK       = 0x01;
        tc.provideAEK    = true;
        tc.includeAccess = true;
        tc.description   = "AEK overrides disable bit";
        return tc;
    }(),*/

    // ===== CRT =====
    []
    {
        auto tc        = util::makePKICRTKeyTest();
        tc.description = "Valid CRT private key import";
        return tc;
    }(),

    []
    {
        auto tc = util::makePKICRTKeyTest();
        tc.q.clear(); // missing q
        tc.expectSuccess = false;
        tc.description   = "Invalid CRT (missing q)";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.p             = util::makeVector(0x20, 0x00); // invalid MSB
        tc.expectSuccess = false;
        tc.description   = "Invalid CRT prime MSB";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.dP            = util::makeVector(0x1F, 0x33); // wrong dP length
        tc.expectSuccess = false;
        tc.description   = "dP length mismatch";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.dQ            = util::makeVector(0x1F, 0x44);
        tc.expectSuccess = false;
        tc.description   = "dQ length mismatch";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.iPQ           = util::makeVector(0x1F, 0x55);
        tc.expectSuccess = false;
        tc.description   = "iPQ length mismatch";
        return tc;
    }(),

    // ===== KEY NUMBER =====
    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.keyNo         = 0x02;
        tc.expectSuccess = false;
        tc.description   = "Private key not allowed at keyNo = 2";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.keyNo         = 0x03;
        tc.n             = util::N512();
        tc.e             = util::E();
        tc.expectSuccess = false;
        tc.description   = "Public key keyNo out of range";
        return tc;
    }(),

    // ===== CONFIG =====
    []
    {
        auto tc        = util::makePKIImportTest();
        tc.config      = pki::DISABLE | pki::CRT;
        tc.n           = util::N512();
        tc.e           = util::E();
        tc.description = "Import disabled key";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKICRTKeyTest();
        tc.config        = pki::PRIVATE_KEY | pki::DISABLE | pki::CRT;
        tc.keyNoAEK      = 0x05;
        tc.keyVAEK       = 0x01;
        tc.provideAEK    = true;
        tc.includeAccess = true;
        tc.description   = "AEK + disabled conflict";
        return tc;
    }(),

    // ===== CHAINING TESTS =====
    []
    {
        auto tc        = util::makePKICRTKeyTest();
        tc.n           = util::N2048();
        tc.e           = ByteVector(256, 0x03);
        tc.p           = util::makeVector(0x80, 0x11);
        tc.q           = util::makeVector(0x80, 0x22);
        tc.dP          = util::makeVector(0x80, 0x33);
        tc.dQ          = util::makeVector(0x80, 0x44);
        tc.iPQ         = util::makeVector(0x80, 0x55);
        tc.description = "Large key forcing APDU chaining";
        return tc;
    }(),

    []
    {
        auto tc          = util::makePKIImportTest();
        tc.config        = pki::PRIVATE_KEY | pki::CRT;
        tc.n             = util::N2048();
        tc.e             = ByteVector(256, 0x04); // even exponent
        tc.expectSuccess = false;
        tc.description   = "Chaining + invalid exponent";
        return tc;
    }()
};

std::vector<PKIGenerateTestCase> generateKeyPairTests = {
    // ===== SUCCESS =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, {}, false, true, false, "Valid parameters, random exponent, minimal modulus"},
    {0x00, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x100, ByteVector(256, 0x03), false, true, false, "APDU chaining with large exponent (no access)"},
    {0x01, pki::PRIVATE_KEY | pki::ALLOW_PRIVATE_EXPORT | pki::CRT, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, {}, false, true, false, "Valid config without access keys"},

    // ===== DISABLED KEY =====
    {0x01, pki::PRIVATE_KEY | pki::ALLOW_PRIVATE_EXPORT | pki::CRT, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, {}, false, true, true, "Key generation with disable flag (bit 2 set)"},

    // ===== MIN EXPONENT SIZE =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, logicalaccess::BufferHelper::fromHexString("00000003"), false, true, false,
     "Minimum valid exponent size (4 bytes and odd)"},

    // ===== EXPONENT (SMALL) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, logicalaccess::BufferHelper::fromHexString("00010001"), false, true, false,
     "Standard RSA exponent (65537)"},

    // ===== CHAINING =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x100, ByteVector(256, 0x03), false, true, false, "APDU chaining (256-byte exponent)"},
    
    // ===== INVALID KEY NUMBER =====
    {0x02, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, {}, false, false, false, "Invalid key number (must be 0x00 or 0x01)"},

    // ===== INCORRECT EXPONENT LENGTH (SMALL) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, logicalaccess::BufferHelper::fromHexString("010001"), false, false, false,
     "Exponent length not multiple of 4 bytes"},

    // ===== INCORRECT EXPONENT (EVEN) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, logicalaccess::BufferHelper::fromHexString("020002"), false, false, false,
     "Exponent must be odd"},

    // ===== AEK (DISABLE CONFLICT) =====
    {0x01, 0x0004, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, {}, true, false, false, "Access keys required but not provided"},

    // ===== INVALID NLEN =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x41, {}, false, false, false, "Modulus length not multiple of 8"},

    // ===== EXPONENT > MODULUS =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x40, ByteVector(128, 0x03), false, false, false, "Exponent length greater than modulus"},

    // ===== AEK WITH DISABLED KEY (CONFLICT) =====
    {0x01, 0x0004, 0xFE, 0x00, 0xFF, sam::AEKVAEK(0x10, 0x20), 0x40, {}, false, false, false, "Access key provided while key is disabled"},

    // ===== NLEN TOO SMALL =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, sam::AEKVAEK{}, 0x20, {},false, false, false, "Modulus length too small (64 bytes required)"}
};

std::vector<PKIUpdateKeyEntriesTestCase> updateKeyEntriesTests = {
    // ===== VALID =====
    {0x00, 0x02, false, 0x00, 0x00, 1, true, "(STABLE) Update one entry SHA1"},
    {0x00, 0x02, true, 0x01, 0x00, 1, true, "Update one entry + ACK"},
    {0x00, 0x02, false, 0x00, 0x01, 2, true, "Update two entries SHA224"},
    {0x00, 0x02, false, 0x00, 0x02, 3, true, "Update three entries SHA256"},

    // ===== ACK =====
    {0x00, 0x02, true, 0x00, 0x02, 3, true, "Update three entries with ACK"},

    // ===== INVALID ENC =====
    {0x02, 0x02, false, 0x00, 0x00, 1, false, "Invalid keyNoEnc"},
    {0xFF, 0x02, false, 0x00, 0x00, 1, false, "Invalid keyNoEnc overflow"},

    // ===== INVALID SIGN =====
    {0x00, 0x03, false, 0x00, 0x00, 1, false, "Invalid keyNoSign"},
    {0x00, 0xFF, false, 0x00, 0x00, 1, false, "Invalid keyNoSign overflow"},

    // ===== INVALID ACK =====
    {0x00, 0x02, true, 0x02, 0x00, 1, false, "Invalid keyNoAck"},
    {0x00, 0x02, true, 0xFF, 0x00, 1, false, "Invalid keyNoAck overflow"},

    // ===== INVALID HASH =====
    {0x00, 0x02, false, 0x00, 0x04, 1, false, "Invalid hash algorithm"},
    {0x00, 0x02, false, 0x00, 0xFF, 1, false, "Invalid hash overflow"},

    // ===== INVALID ENTRY COUNT =====
    {0x00, 0x02, false, 0x00, 0x00, 0, false, "No entries"},
    {0x00, 0x02, false, 0x00, 0x00, 4, false, "More than three entries"},

    // ===== CHAINING =====
    {0x00, 0x02, false, 0x00, 0x02, 3, true, "2048-bit RSA chaining"},
    {0x00, 0x02, true, 0x01, 0x02, 3, true, "2048-bit RSA chaining + ACK"},

    // ===== REPEATABILITY =====
    {0x00, 0x02, false, 0x00, 0x00, 1, true, "Repeat #1"},
    {0x00, 0x02, false, 0x00, 0x00, 1, true, "Repeat #2"},
    {0x00, 0x02, false, 0x00, 0x00, 1, true, "Repeat #3"}
};

std::vector<PKIGenerateHashTestCase> generateHashTests = {
    // ===== VALID =====
    {0x00, ByteVector(16, 0xAA), true, "(STABLE) SHA1 small payload"},
    {0x01, ByteVector(64, 0xAA), true, "SHA224 normal payload"},
    {0x03, ByteVector(128, 0xAA), true, "SHA256 normal payload"},

    // ===== EMPTY DATA =====
    {0x00, {}, false, "SHA1 empty message"},
    {0x01, {}, false, "SHA224 empty message"},
    {0x03, {}, false, "SHA256 empty message"},

    // ===== CHAINING =====
    {0x03, ByteVector(1024, 0xAA), true, "Chaining 1KB payload"},
    {0x03, ByteVector(1536, 0xAA), true, "Chaining 1.5KB payload"},
    {0x03, ByteVector(2048, 0xAA), true, "Chaining 2KB payload"},
    {0x03, ByteVector(3072, 0xAA), true, "Chaining 3KB payload"},

    // ===== BOUNDARY (EXPECTED LIMIT AREA) =====
    {0x03, ByteVector(4096, 0xAA), false, "Chaining boundary (expected failure or limit)"},

    // ===== INVALID HASH =====
    {0x02, ByteVector(32, 0xAA), false, "Unsupported algorithm"},
    {0xFF, ByteVector(32, 0xAA), false, "Hash algorithm overflow"},

    // ===== REPEATABILITY =====
    {0x03, ByteVector(64, 0x11), true, "Repeatability #1"},
    {0x03, ByteVector(64, 0x11), true, "Repeatability #2"}
};

std::vector<PKIGenerateSignatureTestCase> generateSignatureTests = {
    // ===== VALID =====
    {0x00, 0x00, ByteVector(20, 0xAA), true, "(STABLE) SHA1 signature with key #0"},
    {0x01, 0x00, ByteVector(28, 0xAA), true, "SHA224 signature with key #0"},
    {0x03, 0x00, ByteVector(32, 0xAA), true, "SHA256 signature with key #0"},
    {0x03, 0x01, ByteVector(32, 0xAA), true, "SHA256 signature with key #1"},

    // ===== EMPTY DATA =====
    {0x00, 0x00, {}, false, "SHA1 empty hash"},
    {0x03, 0x01, {}, false, "SHA256 empty hash"},

    // ===== HASH SIZE VALIDATION =====
    {0x00, 0x00, ByteVector(19, 0xAA), false, "SHA1 hash too short"},
    {0x00, 0x00, ByteVector(21, 0xAA), false, "SHA1 hash too long"},
    {0x01, 0x00, ByteVector(32, 0xAA), false, "SHA224 wrong hash size"},
    {0x03, 0x00, ByteVector(20, 0xAA), false, "SHA256 wrong hash size"},

    // ===== INVALID HASH ALGORITHM =====
    {0x02, 0x00, ByteVector(32, 0xAA), false, "RFU hash algorithm"},
    {0xFF, 0x00, ByteVector(32, 0xAA), false, "Hash algorithm overflow"},

    // ===== INVALID KEY =====
    {0x03, 0x02, ByteVector(32, 0xAA), false, "Invalid signing key"},
    {0x03, 0xFF, ByteVector(32, 0xAA), false, "Signing key overflow"},

    // ===== REPEATABILITY =====
    {0x03, 0x00, ByteVector(32, 0x55), true, "Repeatability #1"},
    {0x03, 0x00, ByteVector(32, 0x55), true, "Repeatability #2"}
};

std::vector<PKISendSignatureTestCase> sendSignatureTests = {
    // ===== VALID =====
    {true, true, "(STABLE) Retrieve generated signature"},

    // ===== REPEATABILITY =====
    {true, true, "Retrieve signature again"},
    {true, true, "Retrieve signature third time"},

    // ===== STATE DEPENDENCY =====
    {false, false, "No signature available"} //Test will fail if signature present (with previous tests)
};

struct TestStats
{
    int passed = 0;
    int failed = 0;
};

struct TestHooks
{
    std::function<void()> beforeEach;
    std::function<void()> afterEach;
};

template <typename TestCase, typename Runner, typename Reporter>
void runTestSuite(const std::string &suiteName, const std::vector<TestCase> &tests,
                  Runner &&runner, Reporter &&reporter, const TestHooks &hooks = {})
{
    TestStats stats;
    for (const auto &tc : tests)
    {
        std::cout << "\n========================================";
        std::cout << "\n[RUN " << suiteName << " TEST]";
        std::cout << "\n========================================\n";
        if (!RUN_NEGATIVE_TESTS && !tc.expectSuccess)
        {
            std::cout << "[SKIP] " << tc.description << " (negative test disabled)\n";
            continue;
        }
        std::cout << "[CASE] " << tc.description << "\n";

        if (hooks.beforeEach)
            hooks.beforeEach();

        bool success = false;

        try
        {
            auto start = std::chrono::steady_clock::now();
            runner(tc);
            auto end = std::chrono::steady_clock::now();
            success  = true;
            std::cout << "[TIME] "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                      << " ms\n";
        }
        catch (const std::exception &e)
        {
            std::cout << "[EXCEPTION] " << e.what() << "\n";
        }

        if (success == tc.expectSuccess)
        {
            std::cout << "[OK] Result as expected\n";
            stats.passed++;
        }
        else
        {
            std::cout << "[FAIL] Unexpected result\n";
            stats.failed++;
        }

        reporter(tc, success);

        if (hooks.afterEach)
            hooks.afterEach();
    }

    std::cout << "\n========================================\n";
    std::cout << "[" << suiteName << " SUMMARY]\n";
    std::cout << "  Passed : " << stats.passed << "\n";
    std::cout << "  Failed : " << stats.failed << "\n";
    std::cout << "========================================\n";
}

template <typename TestCase, typename Runner>
void runTestSuite(const std::string &suiteName, const std::vector<TestCase> &tests, Runner &&runner)
{
    runTestSuite(suiteName, tests, std::forward<Runner>(runner),
                 [](const TestCase &, bool)
                 { });
}

template <typename TestCase, typename Runner>
void runTestSuite(const std::string &suiteName, const std::vector<TestCase> &tests, Runner &&runner, const TestHooks &hooks)
{
    runTestSuite(suiteName, tests, std::forward<Runner>(runner),
        [](const TestCase &, bool) {},
        hooks);
}

void runPKIGenerateTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("GENERATE KEYPAIR", generateKeyPairTests,
        [&](const PKIGenerateTestCase &tc)
        {
            samCmd->PKI_GenerateKeyPair(tc.keyNo, tc.config, tc.keyNoCEK, tc.keyNoVCEK,
                                        tc.keyNoRef, tc.accessKeys, tc.nLen, tc.pki_e,
                                        tc.includeAccess);
        },
        [&](const PKIGenerateTestCase &tc, bool success)
        {
            if (!success)
                return;
            std::cout << "[INFO] Expected : " << (tc.expectDisabled ? "key DISABLED" : "key ENABLED") << '\n';
        },
        hooks);
}

void runPKIImportTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("IMPORT", importTests,
        [&](const PKIImportTestCase &tc)
        {
            sam::AEKVAEK access(tc.provideAEK ? tc.keyNoAEK : 0,
                                tc.provideAEK ? tc.keyVAEK : 0);
            samCmd->PKI_ImportKey(tc.keyNo, tc.config, tc.keyNoCEK, tc.keyNoVCEK, tc.refNoKUC,
                                  tc.n, tc.e, tc.p, tc.q, tc.dP, tc.dQ, tc.iPQ,
                                  access, tc.includeAccess, tc.updateSettingsOnly);
        },
        hooks);
}

void runPKIExportPrivateTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite(
        "EXPORT PRIVATE", exportPrivateKeyTests,
        [&](const PKIExportPrivateTestCase &tc)
        {
            ByteVector exported = samCmd->PKI_ExportPrivateKey(tc.keyNo, tc.requestAEK);
            if (exported.empty())
                throw std::runtime_error("Empty export payload");
            if (tc.expectPrivatePartPresent && exported.size() < 13) //Private export should be larger than public key export.
                throw std::runtime_error("Private key export unexpectedly small");
        },
        hooks);
}

void runPKIExportPublicTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("EXPORT PUBLIC", exportPublicKeyTests,
        [&](const PKIExportPublicTestCase &tc)
        {
            ByteVector exported = samCmd->PKI_ExportPublicKey(tc.keyNo, tc.returnAEK);
            if (exported.empty())
                throw std::runtime_error("Empty export payload");
            if (exported.size() < 9)
                throw std::runtime_error("Public key export unexpectedly small");
        },
        hooks);
}

void runPKIGenerateHashTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("GENERATE HASH", generateHashTests,
        [&](const PKIGenerateHashTestCase &tc)
        {
            ByteVector hash = samCmd->PKI_GenerateHash(tc.hashAlgo, tc.data);
            if (hash.empty())
                throw std::runtime_error("Empty hash");
        },
        hooks);
}

void runPKIGenerateSignatureTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite(
        "GENERATE SIGNATURE", generateSignatureTests,
        [&](const PKIGenerateSignatureTestCase &tc)
        {
            samCmd->PKI_GenerateSignature(tc.hashAlgo, tc.keyNo, tc.data);
        },
        hooks);
}

void runPKISendSignatureTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("SEND SIGNATURE", sendSignatureTests,
        [&](const PKISendSignatureTestCase &tc)
        {
            if (tc.generateBeforeSend)
            {
                ByteVector hash(28, 0xAA);
                samCmd->PKI_GenerateSignature(0x01, 0x00, hash);
            }
            ByteVector signature = samCmd->PKI_SendSignature();
            if (signature.empty())
                throw std::runtime_error("Empty signature");
        },
        [&](const PKISendSignatureTestCase &, bool success)
        {
            if (success)
                std::cout << "[INFO] Signature successfully retrieved\n";
        },
        hooks);
}

struct PKIVerifySignatureTestCase
{
    uint8_t hashAlgo;
    uint8_t keyNo;
    ByteVector hash;
    bool tamperHash;
    bool tamperSignature;
    bool wrongAlgorithm;
    bool wrongKey;
    bool expectSuccess;
    std::string description;
};

std::vector<PKIVerifySignatureTestCase> verifySignatureTests = {
    // ===== SHA1 =====
    {0x00, 0x01, ByteVector(20, 0xAA), false, false, false, false, true, "(STABLE) SHA1 valid signature"},
    {0x00, 0x01, ByteVector(20, 0x55), false, false, false, false, true, "SHA1 different payload"},
    {0x00, 0x01, ByteVector(20, 0xFF), false, false, false, false, true, "SHA1 all FF hash"},

    // ===== SHA224 =====
    {0x01, 0x01, ByteVector(28, 0xAA), false, false, false, false, true, "(STABLE) SHA224 valid signature"},
    {0x01, 0x01, ByteVector(28, 0x55), false, false, false, false, true, "SHA224 different payload"},
    {0x01, 0x01, ByteVector(28, 0xFF), false, false, false, false, true, "SHA224 all FF hash"},

    // ===== SHA256 =====
    {0x03, 0x01, ByteVector(32, 0xAA), false, false, false, false, true, "(STABLE) SHA256 valid signature"},
    {0x03, 0x01, ByteVector(32, 0x55), false, false, false, false, true, "SHA256 different payload"},
    {0x03, 0x01, ByteVector(32, 0xFF), false, false, false, false, true, "SHA256 all FF hash"},

    // ===== REPEATABILITY =====
    {0x00, 0x01, ByteVector(20, 0x11), false, false, false, false, true, "SHA1 repeat #1"},
    {0x00, 0x01, ByteVector(20, 0x11), false, false, false, false, true, "SHA1 repeat #2"},
    {0x01, 0x01, ByteVector(28, 0x22), false, false, false, false, true, "SHA224 repeat #1"},
    {0x01, 0x01, ByteVector(28, 0x22), false, false, false, false, true, "SHA224 repeat #2"},
    {0x03, 0x01, ByteVector(32, 0x33), false, false, false, false, true, "SHA256 repeat #1"},
    {0x03, 0x01, ByteVector(32, 0x33), false, false, false, false, true, "SHA256 repeat #2"},

    // ===== BOUNDARY CONTENT =====
    {0x00, 0x01, ByteVector(20, 0x00), false, false, false, false, true, "SHA1 all zero hash"},

    {0x01, 0x01, ByteVector(28, 0x00), false, false, false, false, true, "SHA224 all zero hash"},

    {0x03, 0x01, ByteVector(32, 0x00), false, false, false, false, true, "SHA256 all zero hash"},

    {0x00, 0x01,
     []
     {
         ByteVector h(20);
         for (size_t i = 0; i < h.size(); ++i)
             h[i] = static_cast<uint8_t>(i);
         return h;
     }(),
     false, false, false, false, true, "SHA1 incremental pattern"},

    {0x03, 0x01,
     []
     {
         ByteVector h(32);
         for (size_t i = 0; i < h.size(); ++i)
             h[i] = static_cast<uint8_t>(i);
         return h;
     }(),
     false, false, false, false, true, "SHA256 incremental pattern"},

    // ===== WRONG ALGORITHM =====
    {0x00, 0x01, ByteVector(20, 0xAA), false, false, true, false, false, "Verify SHA1 signature as SHA224"},
    {0x01, 0x01, ByteVector(28, 0xAA), false, false, true, false, false, "Verify SHA224 signature as SHA256"},
    {0x03, 0x01, ByteVector(32, 0xAA), false, false, true, false, false, "Verify SHA256 signature as SHA1"},

    // ===== WRONG KEY =====
    {0x03, 0x00, ByteVector(32, 0xAA), false, false, false, true, false, "Verify signature with wrong public key"},
    {0x01, 0x00, ByteVector(28, 0xAA), false, false, false, true, false, "Verify SHA224 with wrong key"},

    // ===== HASH MODIFICATION =====
    {0x00, 0x01, ByteVector(20, 0xAA), true, false, false, false, false, "SHA1 modified hash"},
    {0x01, 0x01, ByteVector(28, 0xAA), true, false, false, false, false, "SHA224 modified hash"},
    {0x03, 0x01, ByteVector(32, 0xAA), true, false, false, false, false, "SHA256 modified hash"},

    // ===== SIGNATURE MODIFICATION =====
    {0x00, 0x01, ByteVector(20, 0xAA), false, true, false, false, false, "SHA1 modified signature"},
    {0x01, 0x01, ByteVector(28, 0xAA), false, true, false, false, false, "SHA224 modified signature"},
    {0x03, 0x01, ByteVector(32, 0xAA), false, true, false, false, false, "SHA256 modified signature"}
};

void runPKIVerifySignatureTests(
    std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd, const TestHooks &hooks)
{
    runTestSuite("VERIFY SIGNATURE", verifySignatureTests,
        [&](const PKIVerifySignatureTestCase &tc)
        {
            samCmd->PKI_GenerateSignature(tc.hashAlgo, tc.keyNo, tc.hash);
            ByteVector hash = tc.hash;
            ByteVector sig  = samCmd->PKI_SendSignature();
            if (sig.empty())
                throw std::runtime_error("Empty signature from SAM (state broken)");
            if (tc.tamperHash && !hash.empty())
                hash[0] ^= 0xFF;
            if (tc.tamperSignature && !sig.empty())
                sig[0] ^= 0xFF;
            uint8_t verifyAlgo = tc.hashAlgo;
            uint8_t verifyKey  = tc.keyNo;
            if (tc.wrongAlgorithm)
            {
                if (tc.hashAlgo == 0x00)
                    verifyAlgo = 0x01;
                else if (tc.hashAlgo == 0x01)
                    verifyAlgo = 0x03;
                else
                    verifyAlgo = 0x00;
            }
            if (tc.wrongKey)
                verifyKey = (tc.keyNo == 0x00) ? 0x01 : 0x00;
            samCmd->PKI_VerifySignature(verifyAlgo, verifyKey, hash, sig);
        },
        hooks);
}

struct PKIEncipherDataTestCase
{
    uint8_t hashAlgo;
    uint8_t keyNoEnc;
    ByteVector plainData;
    bool expectSuccess;
    std::string description;
};

std::vector<PKIEncipherDataTestCase> encipherDataTests = {
    // ===== VALID =====
    {0x00, 0x00, ByteVector(8, 0xAA),  true, "(STABLE) SHA1 OAEP small payload"},
    {0x01, 0x00, ByteVector(8, 0xAA),  true, "SHA224 OAEP small payload"},
    {0x03, 0x00, ByteVector(8, 0xAA),  true, "SHA256 OAEP small payload"},
    {0x00, 0x01, ByteVector(16, 0x55), true, "SHA1 with key #1"},
    {0x03, 0x02, ByteVector(16, 0x11), true, "SHA256 with key #2"},

    // ===== BOUNDARY DATA =====
    {0x00, 0x00, ByteVector(1, 0xAA),  true, "Minimum payload"},
    {0x03, 0x00, ByteVector(2, 0xAA),  true, "Very small payload"},
    {0x03, 0x00, ByteVector(32, 0xAA), true, "32-byte payload"},

    // ===== CONTENT VARIATIONS =====
    {0x00, 0x00, ByteVector(16, 0x00), true, "All zero plaintext"},
    {0x01, 0x00, ByteVector(16, 0xFF), true, "All FF plaintext"},
    {0x03, 0x00,
     []{
         ByteVector v(32);
         for (size_t i = 0; i < v.size(); ++i)
             v[i] = static_cast<uint8_t>(i);
         return v;
     }(),
     true,
     "Incremental plaintext"},

    // ===== EMPTY DATA =====
    {0x00, 0x00, {}, false, "Empty plaintext"},
    {0x01, 0x00, {}, false, "Empty plaintext SHA224"},
    {0x03, 0x00, {}, false, "Empty plaintext SHA256"},

    // ===== INVALID HASH ALGO =====
    {0x02, 0x00, ByteVector(8, 0xAA), false, "RFU hash algorithm"},
    {0xFF, 0x00, ByteVector(8, 0xAA), false, "Hash algorithm overflow"},

    // ===== INVALID KEY =====
    {0x00, 0x03, ByteVector(8, 0xAA), false, "Invalid key number"},
    {0x03, 0xFF, ByteVector(8, 0xAA), false, "Key number overflow"},

    // ===== REPEATABILITY =====
    {0x03, 0x00, ByteVector(16, 0x77), true, "Repeatability #1"},
    {0x03, 0x00, ByteVector(16, 0x77), true, "Repeatability #2"}
};

void runPKIEncipherDataTests(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("ENCIPHER DATA", encipherDataTests,
        [&](const PKIEncipherDataTestCase &tc)
        {
            ByteVector encrypted = samCmd->PKI_EncipherData(tc.hashAlgo, tc.keyNoEnc, tc.plainData);
            if (encrypted.empty())
                throw std::runtime_error("Empty encrypted data");
            std::cout << "[ENC SIZE] " << encrypted.size() << " bytes\n";
        },
        hooks);
}

struct PKIDecipherDataTestCase
{
    uint8_t hashAlgo;
    uint8_t keyNoDec;
    ByteVector encData;
    bool expectSuccess;
    std::string description;
};

std::vector<PKIDecipherDataTestCase> decipherDataTests = {
    // ===== VALID =====
    {0x00, 0x00, ByteVector(128, 0xAA), true, "(STABLE) SHA1 decrypt key #0"},
    {0x01, 0x00, ByteVector(128, 0xAA), true, "SHA224 decrypt key #0"},
    {0x03, 0x00, ByteVector(128, 0xAA), true, "SHA256 decrypt key #0"},

    {0x00, 0x01, ByteVector(128, 0x55), true, "SHA1 decrypt key #1"},
    {0x03, 0x01, ByteVector(128, 0x11), true, "SHA256 decrypt key #1"},

    // ===== CIPHERTEXT CONTENT =====
    {0x00, 0x00, ByteVector(128, 0x00), true, "All zero ciphertext"},
    {0x00, 0x00, ByteVector(128, 0xFF), true, "All FF ciphertext"},

    {0x03, 0x00,
     []()
     {
         ByteVector v(128);
         for (size_t i = 0; i < v.size(); ++i)
             v[i] = static_cast<uint8_t>(i);
         return v;
     }(),
     true, "Incremental ciphertext"},

    // ===== EMPTY DATA =====
    {0x00, 0x00, {}, false, "Empty ciphertext"},
    {0x01, 0x00, {}, false, "Empty ciphertext SHA224"},
    {0x03, 0x00, {}, false, "Empty ciphertext SHA256"},

    // ===== INVALID HASH =====
    {0x02, 0x00, ByteVector(128, 0xAA), false, "RFU hash algorithm"},
    {0xFF, 0x00, ByteVector(128, 0xAA), false, "Hash algorithm overflow"},

    // ===== INVALID KEY =====
    {0x00, 0x02, ByteVector(128, 0xAA), false, "Invalid key number"},
    {0x00, 0xFF, ByteVector(128, 0xAA), false, "Key number overflow"},

    // ===== WRONG RSA SIZE =====
    {0x00, 0x00, ByteVector(64, 0xAA), false, "Ciphertext too short"},
    {0x00, 0x00, ByteVector(127, 0xAA), false, "Ciphertext one byte short"},
    {0x00, 0x00, ByteVector(129, 0xAA), false, "Ciphertext one byte too long"},
    {0x00, 0x00, ByteVector(256, 0xAA), false, "Ciphertext too large"},

    // ===== REPEATABILITY =====
    {0x03, 0x00, ByteVector(128, 0x77), true, "Repeatability #1"},
    {0x03, 0x00, ByteVector(128, 0x77), true, "Repeatability #2"}
};

void runPKIDecipherDataTests(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("DECIPHER DATA", decipherDataTests,
        [&](const PKIDecipherDataTestCase &tc)
        {
            ByteVector plain = samCmd->PKI_DecipherData(tc.hashAlgo, tc.keyNoDec, tc.encData);
            if (plain.empty())
                throw std::runtime_error("Empty plaintext");
            std::cout << "[PLAIN SIZE] " << plain.size() << " bytes\n";
        },
        hooks);
}

struct PKIRoundTripTestCase
{
    uint8_t hashAlgo;
    uint8_t keyNo;
    ByteVector plainData;
    bool expectSuccess;
    std::string description;
};

std::vector<PKIRoundTripTestCase> roundTripTests = {
    // ===== STABLE =====
    {0x00, 0x00, ByteVector(8, 0xAA), true, "(STABLE) SHA1 roundtrip"},
    {0x01, 0x00, ByteVector(8, 0xAA), true, "(STABLE) SHA224 roundtrip"},
    {0x03, 0x00, ByteVector(8, 0xAA), true, "(STABLE) SHA256 roundtrip"},

    // ===== CONTENT =====
    {0x03, 0x00, ByteVector(1, 0x00), true, "Single byte"},
    {0x03, 0x00, ByteVector(16, 0x00), true, "All zero plaintext"},
    {0x03, 0x00, ByteVector(16, 0xFF), true, "All FF plaintext"},
    {0x03, 0x00,
     []
     {
         ByteVector v(32);
         for (size_t i = 0; i < v.size(); ++i)
             v[i] = static_cast<uint8_t>(i);
         return v;
     }(),
     true, "Incremental pattern"},

    // ===== REPEATABILITY =====
    {0x03, 0x00, ByteVector(16, 0x55), true, "Repeatability #1"},
    {0x03, 0x00, ByteVector(16, 0x55), true, "Repeatability #2"},
};

void runPKIRoundTripTests(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("PKI ROUNDTRIP", roundTripTests,
        [&](const PKIRoundTripTestCase &tc)
        {
            std::cout << "[ENCIPHER]";
            ByteVector encrypted = samCmd->PKI_EncipherData(tc.hashAlgo, tc.keyNo, tc.plainData);
            
            // Test OAEP randomness
            // ByteVector encrypted2 = samCmd->PKI_EncipherData(tc.hashAlgo, tc.keyNo, tc.plainData);
            // if (encrypted == encrypted2)
            //     throw std::runtime_error("OAEP randomness failure");
            if (encrypted.empty())
                throw std::runtime_error("Empty ciphertext");
            // ByteVector decrypted2 = samCmd->PKI_DecipherData(tc.hashAlgo,
            // tc.keyNo, encrypted); if (decrypted != tc.plainData || decrypted2 != tc.plainData)
            //      throw std::runtime_error("Roundtrip mismatch");
            ByteVector decrypted = samCmd->PKI_DecipherData(tc.hashAlgo, tc.keyNo, encrypted);
            if (decrypted != tc.plainData)
                throw std::runtime_error("Recovered plaintext differs from original");
        },
        hooks);
}

struct PKIImportEccTestCase
{
    uint8_t keyNo;
    uint16_t eccSet;
    uint8_t keyNoCEK;
    uint8_t keyNoVCEK;
    uint8_t keyNoKUC;
    uint8_t keyNoAEK;
    uint8_t keyNoVAEK;
    ByteVector eccPublicKey;
    bool settingsOnly;
    bool expectSuccess;
    std::string description;
};

ByteVector makeSafeEccPoint()
{
    ByteVector ecc;
    ecc.reserve(33);
    ecc.push_back(0x04);
    for (uint8_t i = 0; i < 32; i++)
        ecc.push_back(i);
    return ecc;
}

static const ByteVector SAFE_ECC_POINT = {
    0x04,
    // X (32 bytes)
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
    0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
    // Y (32 bytes)
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
    0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44
};

std::vector<PKIImportEccTestCase> importEccKeyTests = {
    {0x07, 0x0000, 0xFE, 0x00, 0xFF, 0xFE, 0x00, {}, true, true, "Settings-only update (NO key material)"},
    
    {0x07, 0x0000, 0xFE, 0x00, 0xFF, 0xFE, 0x00, SAFE_ECC_POINT,
    false, true, "Full import (synthetic ECC point)"},

    {0x07, 0x0000, 0xFE, 0x00, 0xFF, 0xFE, 0x00,
     ByteVector{0x04, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10},
     false, true, "Minimal ECC curve configuration"}
};

void runPKIImportEccTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("IMPORT ECC KEY", importEccKeyTests,
        [&](const PKIImportEccTestCase &tc)
        {
            samCmd->PKI_ImportEccKey(tc.keyNo, tc.eccSet, tc.keyNoCEK, tc.keyNoVCEK, tc.keyNoKUC,
                tc.keyNoAEK, tc.keyNoVAEK, tc.eccPublicKey, tc.settingsOnly
            );
        },
        [&](const PKIImportEccTestCase &tc, bool success)
        {
            std::cout << "[INFO] " << tc.description << " -> " << (success ? "OK" : "FAIL") << '\n';
            if (!success && tc.expectSuccess)
                throw std::runtime_error("Unexpected ECC import failure: " + tc.description);
            if (success && !tc.expectSuccess)
                throw std::runtime_error("Unexpected ECC import success: " + tc.description);
        },
        hooks);
}

struct PKIImportEccCurveTestCase
{
    unsigned char curveNo;
    unsigned char keyNoCCK;
    unsigned char keyNoVCCK;
    ByteVector curve;
    bool settingsOnly;
    bool expectSuccess;
    std::string description;
};
struct ECCCurveBuilder
{
    static ByteVector BuildFakeCurve(uint8_t eccN, uint8_t eccM, uint8_t fillPrime,
                                     uint8_t fillA, uint8_t fillB, uint8_t fillPx,
                                     uint8_t fillPy, uint8_t fillOrder)
    {
        ByteVector v;
        v.reserve(2 + 5 * eccN + eccM);
        v.push_back(eccN);
        v.push_back(eccM);
        auto appendN = [&](uint8_t val) { v.insert(v.end(), eccN, val); };
        appendN(fillPrime);
        appendN(fillA);
        appendN(fillB);
        appendN(fillPx);
        appendN(fillPy);
        v.insert(v.end(), eccM, fillOrder);
        return v;
    }
};

std::vector<PKIImportEccCurveTestCase> ImportEccCurveTests = {
    {0x00, 0xFE, 0x00,
     ECCCurveBuilder::BuildFakeCurve(0x10,
                                     0x10,
                                     0x11, 0x22, 0x33, 0x44, 0x55, 0x66),
     false, true, "Valid ECC curve (minimal safe size)"}};


void runPKIImportEccCurve(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks& hooks)
{
    runTestSuite("IMPORT ECC CURVE", ImportEccCurveTests,
        [&](const PKIImportEccCurveTestCase &tc)
        {
            samCmd->PKI_ImportEccCurve(tc.curveNo, tc.keyNoCCK, tc.keyNoVCCK, tc.curve, tc.settingsOnly);
        },
        hooks);
}

struct PKIExportEccKeyTestCase
{
    unsigned char keyNo;
    bool expectSuccess;
    std::string description;
};

static std::vector<PKIExportEccKeyTestCase> exportEccKeyTests = {
    {0x00, true, "Export ECC key 0 (safe baseline)"},
    {0x01, true, "Export ECC key 1 (safe baseline)"},
    {0x02, true, "Export ECC key 2 (safe baseline)"}
};

void runPKIExportEccPublicKey(std::shared_ptr<SAMAV3ISO7816Commands> samCmd, const TestHooks &hooks)
{
    runTestSuite("EXPORT ECC PUBLIC KEY", exportEccKeyTests,
        [&](const PKIExportEccKeyTestCase &tc)
        {
            ByteVector exported = samCmd->PKI_ExportEccPublicKey(tc.keyNo);
            if (exported.empty())
                throw std::runtime_error("[FAIL] Empty export result\n");
        },
        hooks);
}

struct PKIVerifyEccSignatureTestCase
{
    unsigned char keyNo{};
    unsigned char curveNo{};
    ByteVector message{};
    ByteVector signature{};
    bool expectSuccess{};
    std::string description;
};

std::vector<PKIVerifyEccSignatureTestCase> verifyEccSignatureTests = {
    {0x00, 0x00, ByteVector{0x01}, ByteVector(64, 0xAA), true, "Minimum valid message"},
    {0x07, 0x03, ByteVector(32, 0x11), ByteVector(64, 0x55), true, "Maximum key and curve numbers"},
    {0x00, 0x00, ByteVector(255, 0x22), ByteVector(64, 0x77), true, "Maximum protocol message length"},
    {0x08, 0x00, ByteVector{0x01}, ByteVector(64, 0xAA), false, "Invalid key number"},
    {0x00, 0x04, ByteVector{0x01}, ByteVector(64, 0xAA), false, "Invalid curve number"},
    {0x00, 0x00, {}, ByteVector(64, 0xAA), false, "Empty message"},
    {0x00, 0x00, ByteVector(256, 0x11), ByteVector(64, 0xAA), false, "Message length exceeds protocol limit"},
    {0x00, 0x00, ByteVector{0x01}, {}, false, "Empty signature"}
};

void runPKIVerifyEccSignatureTests(std::shared_ptr<SAMAV3ISO7816Commands> samCmd,
                                   const TestHooks &hooks)
{
    runTestSuite("VERIFY ECC SIGNATURE", verifyEccSignatureTests,
        [&](const PKIVerifyEccSignatureTestCase &tc)
        {
            samCmd->PKI_VerifyEccSignature(tc.keyNo, tc.curveNo, tc.message, tc.signature);
        },
        [&](const PKIVerifyEccSignatureTestCase &, bool) {}, hooks);
}

struct ReaderSession
{
    std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig;
    std::shared_ptr<logicalaccess::ReaderUnit> reader;
    std::shared_ptr<logicalaccess::Chip> chip;

    ~ReaderSession()
    {
        if (reader)
        {
            try
            {
                reader->disconnect();
            }
            catch (...)
            {
            }
        }
    }

    void connect()
    {
        reader = readerConfig->getReaderUnit();
        reader->setCardType("SAM_AV3");
        reader->connectToReader();
    }

    void waitCard()
    {
        if (!reader->waitInsertion(15000))
            throw std::runtime_error("[INFO] No card inserted (timeout)");
        if (!reader->connect())
            throw std::runtime_error("[ERROR] Failed to connect to card");
        std::cout << "Card inserted on reader : " << reader->getConnectedName() << std::endl;
        chip = reader->getSingleChip();
        if (!chip)
            throw std::runtime_error("[ERROR] No chip detected");
    }

    void waitRemovalSafe()
    {
        if (!reader->waitRemoval(3000))
            std::cerr << "[WARN] Card removal forced" << std::endl;
    }
};

struct SamSession
{
    std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd;
    std::shared_ptr<logicalaccess::DESFireKey> hostKey;

    void attach(const std::shared_ptr<logicalaccess::Chip> &chip)
    {
        samCmd = std::dynamic_pointer_cast<logicalaccess::SAMAV3ISO7816Commands>(chip->getCommands());
        std::cout << "[INFO] Commands type : " << typeid(*chip->getCommands()).name() << std::endl;
        if (!samCmd)
            throw std::runtime_error("SAMAV3Commands not available");
    }

    void authenticate()
    {
        if (!hostKey)
        {
            hostKey = std::make_shared<logicalaccess::DESFireKey>();
            hostKey->setKeyType(logicalaccess::DF_KEY_AES);
            hostKey->setData(logicalaccess::BufferHelper::fromHexString(
                "00000000000000000000000000000000"));
        }
        samCmd->SAMAV2ISO7816Commands::authenticateHost(hostKey, 0x00);
    }
};

int main(int, char **)
{
    try
    {
        auto readerConfig = std::make_shared<logicalaccess::ReaderConfiguration>();
        const std::string providerName = "PCSC";
        readerConfig->setReaderProvider(logicalaccess::LibraryManager::getInstance()->getReaderProvider(providerName));
        if (!readerConfig->getReaderProvider())
            throw std::runtime_error("No PCSC reader provider available");
        auto readers = readerConfig->getReaderProvider()->getReaderList();
        if (readers.empty())
            throw std::runtime_error("No PCSC readers detected");
        readerConfig->setReaderUnit(readers.at(0));
        ReaderSession readerSession;
        SamSession samSession;
        readerSession.readerConfig = readerConfig;
        std::cout << "Waiting 15 seconds for card insertion..." << std::endl;
        readerSession.connect();
        std::cout << "[INFO] Time start : " << time(NULL) << std::endl;
        readerSession.waitCard();
        std::cout << "[INFO] Card type : " << readerSession.chip->getCardType() << std::endl;
        samSession.attach(readerSession.chip);
        std::cout << "[AUTH] Performing SAM Host Authentication..." << std::endl;
        samSession.authenticate();
        std::cout << "[AUTH] OK" << std::endl;
        TestHooks samHooks;
        samHooks.afterEach = [&]()
        {
            if (!REAUTH_AFTER_EACH_TEST)
                return;
            std::cout << "\n[HOOK] Resetting SAM session..." << std::endl;
            try
            {
                readerSession.reader->disconnect();
                std::cout << "[HOOK] Reader disconnected" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cout << "[HOOK] Disconnect failed: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cout << "[HOOK] Disconnect failed : unknown error" << std::endl;
            }
            readerSession.reader->connect();
            std::cout << "[HOOK] Reader connected" << std::endl;
            samSession.attach(readerSession.reader->getSingleChip());
            std::cout << "[HOOK] Re-authenticating SAM..." << std::endl;
            samSession.authenticate();
            std::cout << "[HOOK] SAM authenticated" << std::endl;
        };
        for (int session = 0; session < 1; ++session) //Will be improved later
        {
            std::cout << "\n=== TEST SESSION " << session << " ===\n";

            // Some tests require a specific PKI_SET configuration and scenario setup
            // and cannot be executed in a purely isolated function context.

            //runPKIGenerateTests(samSession.samCmd, samHooks);
            //runPKIExportPrivateTests(samSession.samCmd, samHooks);
            //runPKIGenerateHashTests(samSession.samCmd, samHooks);
        }
        std::cout << "\nLogical automatic card removal in 3 seconds...\n";
        readerSession.waitRemovalSafe();
        std::cout << "Card removed." << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[FATAL] " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}