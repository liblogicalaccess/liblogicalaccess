#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav3iso7816commands.hpp>

using namespace logicalaccess;

constexpr uint16_t PKI_PUBLIC  = 0;
constexpr uint16_t PKI_PRIVATE = 1 << 0;
constexpr uint16_t PKI_EXPORT = 1 << 1;
constexpr uint16_t PKI_DISABLE = 1 << 2;
constexpr uint16_t PKI_CRT = 1 << 6;

ByteVector makeVector(size_t size, unsigned char value = 0x01)
{
    return ByteVector(size, value);
}

ByteVector defaultE()
{
    return logicalaccess::BufferHelper::fromHexString("00010001");
}

struct PKIGenerateTestCase
{
    unsigned char keyNo;
    unsigned short config;
    unsigned char keyNoCEK;
    unsigned char keyNoVCEK;
    unsigned char keyNoRef;
    bool includeAccess;
    unsigned short nLen;
    ByteVector pki_e;
    bool expectSuccess;
    bool expectDisabled;
    std::string description;
};

struct PKIImportTestCase
{
    unsigned char keyNo;
    unsigned short config;
    unsigned char keyNoCEK;
    unsigned char keyNoVCEK;
    unsigned char refNoKUC;
    ByteVector n;
    ByteVector e;
    ByteVector p;
    ByteVector q;
    ByteVector dP;
    ByteVector dQ;
    ByteVector iPQ;
    bool includeAccess;
    bool expectSuccess;
    std::string description;
};

struct ExportTestCase
{
    uint8_t keyNo;
    bool requestAEK;
    bool expectSuccess;
    bool expectsPrivateKeyPresent;
    std::string desc;
};

struct ExportPublicKeyTestCase
{
    uint8_t keyNo;
    bool returnAEK;
    bool expectSuccess;
    std::string description;
};


std::vector<ExportPublicKeyTestCase> exportPublicKeyTests = {
    // ===== SUCCESS =====
    {0x00, false, true, "Valid export keyNo = 0 without AEK"},
    {0x01, false, true, "Valid export keyNo = 1 without AEK"},
    {0x02, false, true, "Valid export keyNo = 2 boundary valid"},

    {0x00, true, true, "Valid export keyNo = 0 with AEK"},
    {0x01, true, true, "Valid export keyNo = 1 with AEK"},
    {0x02, true, true, "Valid export keyNo = 2 with AEK"},

    // ===== INVALID KEY REFERENCES =====
    {0x03, false, false, "Invalid keyNo out of range (0x03)"},
    {0x10, false, false, "Invalid keyNo mid-range invalid"},
    {0xFF, false, false, "Invalid keyNo overflow"},

    // ===== AEK =====
    {0x00, true, true, "AEK request valid when allowed"},
    {0x01, true, true, "AEK request valid key 1"},
    {0x02, true, true, "AEK request valid key 2"},

    // ===== SECURITY / CONFIG RESTRICTIONS =====
    {0x00, false, false, "Export denied by PKI_SET configuration"},
    {0x01, false, false, "Export blocked (private key only slot or disabled)"},
    {0x02, false, false, "Export restricted key slot"},

    // ===== BLOCKING RULES =====
    {0x00, false, false, "Private key export not allowed (expected 6986)"},
    {0x01, false, false, "Private-only slot blocked"},
    {0x02, false, false, "Mixed slot blocked"},

    // ===== CHAINING =====
    {0x00, false, true, "Large key export triggers chaining (0x90AF)"},
    {0x01, false, true, "Large export stable chaining"},
    {0x02, true, true, "Large export with AEK chaining"},

    // ===== SAM STATE DEPENDENCY / RESILIENCE =====
    {0x00, false, true, "Repeatability test 1"},
    {0x00, false, true, "Repeatability test 2"},
    {0x01, false, true, "Repeatability stability test"},

    // ===== ERROR =====
    {0x00, false, false, "SAM locked state (6985) expected"},
    {0x00, true, false, "AEK denied (6986) expected"},
    {0x02, true, false, "Host protection error (6A84 possible)"},

    // ===== EDGE CASES =====
    {0x00, false, false, "APDU framing error simulation (6A86)"},
    {0x01, false, false, "Invalid Le handling (6C00)"},
    {0x02, false, false, "Wrong length response (6700)"},

    // ===== ROBUSTNESS =====
    {0x00, false, true, "Stress export key 0 repeated"},
    {0x00, false, true, "Stress export key 0 repeated 2"},
    {0x01, false, true, "Stress export key 1 repeated"},
};

std::vector<ExportTestCase> exportTests = {

    // ===== SUCCESS =====
    {0x00, false, true, true, "Export key 0 basic"},
    {0x01, false, true, true, "Export key 1 basic"},
    {0x00, true, true, true, "Export with AEK"},

    // ===== SECURITY =====
    {0x00, false, false, false, "No private key present"},
    {0x01, false, false, false, "Blocked by config"},
    {0x02, false, false, false, "Key slot invalid"},

    // ===== PERMISSION =====
    {0x00, true, false, true, "AEK requested but not allowed"},

    // ===== BOUNDARIES =====
    {0xFF, false, false, false, "Overflow keyNo"},
    {0x02, false, false, false, "Out of range"},

    // ===== STABILITY =====
    {0x00, false, true, true, "Repeatability test"},
    {0x00, false, true, true, "Repeatability test 2"}

};

std::vector<PKIImportTestCase> importTests = {

    // ===== SUCCESS =====
    {0x01, PKI_PUBLIC | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x80, 0xBB), defaultE(), {}, {}, {}, {}, {}, false, true, "Valid public key import (1024-bit)"},

    // ===== INVALID RSA =====
    {0x01, PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x41, 0xAA), defaultE(), {}, {}, {}, {}, {}, false, false, "Invalid modulus length (not multiple of 8)"},

    {0x01, PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0x00), defaultE(), {}, {}, {}, {}, {}, false, false, "Invalid modulus MSB = 0"},

    {0x01, PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), logicalaccess::BufferHelper::fromHexString("020002"), {}, {}, {}, {}, {}, false, false, "Exponent even"},

    {0x01, PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), logicalaccess::BufferHelper::fromHexString("010001"), {}, {}, {}, {}, {}, false, false, "Exponent too small (3 bytes)"},
    
    {0x01, PKI_DISABLE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(), {}, {}, {}, {}, {}, true, true, "AEK overrides disable bit (must succeed)"},

    {0x01, PKI_DISABLE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(), {}, {}, {}, {}, {}, false, true, "settings-only ignore disable bit correctly"},

    // ===== CRT =====

    {0x01, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(),
     makeVector(0x20, 0x11), makeVector(0x20, 0x22), makeVector(0x20, 0x33),
     makeVector(0x20, 0x44), makeVector(0x20, 0x55), false, true,
     "Valid CRT private key import"},

    {0x01, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA),
     defaultE(), makeVector(0x20, 0x11), {}, // missing q
     makeVector(0x20, 0x33), makeVector(0x20, 0x44), makeVector(0x20, 0x55), false, false, "Invalid CRT (missing q)"},

    {0x01, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(),
     makeVector(0x20, 0x00), // invalid MSB
     makeVector(0x20, 0x22), makeVector(0x20, 0x33), makeVector(0x20, 0x44),
     makeVector(0x20, 0x55), false, false, "Invalid CRT prime MSB"},

    // ===== KEY NUMBER =====

    {0x02, PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(), {}, {}, {}, {}, {}, false, true, "Valid public key at keyNo=2"},

    {0x02, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(),
     makeVector(0x20, 0x11), makeVector(0x20, 0x22), makeVector(0x20, 0x33),
     makeVector(0x20, 0x44), makeVector(0x20, 0x55), false, false,
     "Private key not allowed at keyNo = 2"},

    // ===== CONFIG =====

    {0x01, PKI_DISABLE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(), {}, {}, {}, {}, {}, false, true, "Import disabled key"},

    {0x01, PKI_PRIVATE | PKI_DISABLE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x40, 0xAA), defaultE(), {}, {}, {}, {}, {}, true, false, "AEK + disabled conflict"},

    // ===== CHAINING TESTS =====

    {0x01, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF,
     makeVector(0x100, 0xAA), // 2048-bit
     ByteVector(256, 0x03), makeVector(0x80, 0x11), makeVector(0x80, 0x22),
     makeVector(0x80, 0x33), makeVector(0x80, 0x44), makeVector(0x80, 0x55), false,
     true, "Large key forcing APDU chaining"},

    {0x01, PKI_PRIVATE | PKI_CRT, 0xFE, 0x00, 0xFF, makeVector(0x100, 0xAA),
     ByteVector(256, 0x04), // even exponent
     {}, {}, {}, {}, {}, false, false, "Chaining + invalid exponent"}
     
};

std::vector<PKIGenerateTestCase> tests = { // generateKeyPair tests
    
    // ===== SUCCESS =====
     {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40, {}, true, false,
     "Valid parameters, random exponent, minimal modulus"},

    // ===== DISABLED KEY =====
    {0x01, 0x0004, 0xFE, 0x00, 0xFF, false, 0x40, {}, true, true,
    "Key generation requested with disable flag (bit 2 set)"},

    // ===== INVALID KEY NUMBER =====
    {0x02, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40, {}, false, false,
    "Invalid key number (must be 0x00 or 0x01)"},

    // ===== INCORRECT EXPONENT LENGTH (SMALL) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40,
     logicalaccess::BufferHelper::fromHexString("010001"), false, false,
     "Exponent length not multiple of 4 bytes"},

    // ===== MIN EXPONENT SIZE =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40,
     logicalaccess::BufferHelper::fromHexString("00000003"), true, false,
     "Minimum valid exponent size (4 bytes and odd)"},

    // ===== EXPONENT (SMALL) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40,
     logicalaccess::BufferHelper::fromHexString("00010001"), true, false,
     "Standard RSA exponent 65537 (4 bytes)"},

    // ===== CHAINING =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x100, ByteVector(256, 0x03), true, false,
     "APDU chaining (256-byte exponent)"},

    // ===== INCORRECT EXPONENT (EVEN) =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40,
     logicalaccess::BufferHelper::fromHexString("020002"), false, false,
     "Exponent must be odd"},

    // ===== AEK (DISABLE CONFLICT) =====
    {0x01, 0x0004, 0xFE, 0x00, 0xFF, true, 0x40, {}, false, false,
     "Access key provided while key entry is disabled"},

    // ===== INVALID NLEN =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x41, {}, false, false,
     "Modulus length not multiple of 8"},

    // ===== EXPONENT > MODULUS =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x40, ByteVector(128, 0x03), false, false,
    "Exponent length greater than modulus"},

    // ===== AEK WITH DISABLED KEY (CONFLICT) =====
    {0x01, 0x0004, 0xFE, 0x00, 0xFF, true, 0x40, {}, false, false, "Access key provided while key is disabled"},

    // ===== NLEN TOO SMALL =====
    {0x01, 0x0001, 0xFE, 0x00, 0xFF, false, 0x20, {}, false, false, "Modulus length too small (64 bytes required)"}

};

void runPKITestGenerate(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd,
                        ReaderUnitPtr reader, std::vector<PKIGenerateTestCase> &tc)
{
    int passed = 0;
    int failed = 0;

    for (const auto &tc : tests)
    {
        std::cout << "\n========================================";
        std::cout << "\n[RUN TEST] keyNo = " << (int)tc.keyNo;
        std::cout << "\n========================================\n";

        /*if (tc.keyNo > 0x01) //Old safeguard
        {
            std::cout << "[SKIP] Only keys 0 and 1 allowed\n";
            continue;
        }*/

        bool success = false;

        try
        {
            auto start = std::chrono::steady_clock::now();

            samCmd->PKI_GenerateKeyPair(tc.keyNo, tc.config, tc.keyNoCEK, tc.keyNoVCEK,
                                        tc.keyNoRef, nullptr, nullptr, tc.nLen, tc.pki_e,
                                        tc.includeAccess);

            auto end = std::chrono::steady_clock::now();
            reader->connectToReader(); // for large tests

            std::cout << "[TIME] "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                               start)
                             .count()
                      << " ms\n";

            success = true;
        }
        catch (const std::exception &e)
        {
            std::cout << "[EXCEPTION] " << e.what() << std::endl;
        }

        if (success == tc.expectSuccess)
        {
            std::cout << "[OK] Result as expected\n";
            passed++;
        }
        else
        {
            std::cerr << "[FAIL] Unexpected result\n";
            failed++;
        }

        if (success)
        {
            if (tc.expectDisabled)
                std::cout << "[INFO] Expected : key DISABLED\n";
            else
                std::cout << "[INFO] Expected : key ENABLED\n";
        }
    }
    std::cout << "\n========================================\n";
    std::cout << "[SUMMARY]\n";
    std::cout << "  Passed : " << passed << "\n";
    std::cout << "  Failed : " << failed << "\n";
    std::cout << "========================================\n";

}

void runPKIImportTest(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd,
                      ReaderUnitPtr reader,
                      std::shared_ptr<logicalaccess::DESFireKey> key,
                      const std::vector<PKIImportTestCase> &tc)
{
    int passed = 0;
    int failed = 0;

    for (const auto &tc : importTests)
    {
        std::cout << "\n========================================";
        std::cout << "\n[RUN IMPORT TEST]";
        std::cout << "\n========================================\n";

        if (tc.keyNo > 0x02)
        {
            std::cout << "[SKIP] keyNo out of supported range\n";
            continue;
        }
        bool success = false;

        try
        {
            auto start = std::chrono::steady_clock::now();

            samCmd->PKI_ImportKey(tc.keyNo, tc.config, tc.keyNoCEK, tc.keyNoVCEK,
                                  tc.refNoKUC, tc.n, tc.e, tc.p, tc.q, tc.dP, tc.dQ,
                                  tc.iPQ, nullptr, nullptr, tc.includeAccess, false);

            auto end = std::chrono::steady_clock::now();

            std::cout << "[TIME] "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                               start)
                             .count()
                      << " ms\n";

            success = true;

            // for large APDUs (chaining stability)
            reader->connectToReader();
        }
        catch (const std::exception &e)
        {
            std::cout << "[EXCEPTION] " << e.what() << std::endl;
        }

        if (success == tc.expectSuccess)
        {
            std::cout << "[OK] Result as expected\n";
            passed++;
        }
        else
        {
            std::cerr << "[FAIL] Unexpected result\n";
            failed++;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "[IMPORT SUMMARY]\n";
    std::cout << "  Passed : " << passed << "\n";
    std::cout << "  Failed : " << failed << "\n";
    std::cout << "========================================\n";
}

int main(int, char **)
{
    try
    {

        std::shared_ptr<logicalaccess::ReaderConfiguration> readerConfig(
            new logicalaccess::ReaderConfiguration());

        std::string rpstr = "PCSC";
        readerConfig->setReaderProvider(
            logicalaccess::LibraryManager::getInstance()->getReaderProvider(rpstr));

        if (readerConfig->getReaderProvider() == nullptr)
        {
            std::cerr << "No PCSC reader provider available" << std::endl;
            return EXIT_FAILURE;
        }

        const logicalaccess::ReaderList readers =
            readerConfig->getReaderProvider()->getReaderList();

        if (readers.empty())
        {
            std::cerr << "No PCSC readers detected." << std::endl;
            return EXIT_FAILURE;
        }

        readerConfig->setReaderUnit(readers.at(0));
        std::cout << "Waiting 15 seconds for a card insertion..." << std::endl;

        for (int session = 0; session < 1; ++session)
        {
            auto reader = readerConfig->getReaderUnit();
            reader->setCardType("SAM_AV3");
            reader->connectToReader();
            std::cout << "[INFO] Time start : " << time(NULL) << std::endl;
            if (!reader->waitInsertion(15000))
            {
                std::cout << "[INFO] No card inserted (timeout)" << std::endl;
                continue;
            }
            if (!reader->connect())
            {
                std::cerr << "[ERROR] Failed to connect to card" << std::endl;
                continue;
            }
            std::cout << "Card inserted on reader : " << reader->getConnectedName()
                      << std::endl;
            auto chip = reader->getSingleChip();
            if (!chip)
            {
                std::cerr << "[ERROR] No chip detected" << std::endl;
                continue;
            }
            std::cout << "[INFO] Card type : " << chip->getCardType() << std::endl;

            auto samCmd = std::dynamic_pointer_cast<logicalaccess::SAMAV3ISO7816Commands>(
                chip->getCommands());

            std::cout << "[INFO] Commands type : " << typeid(*chip->getCommands()).name()
                      << std::endl;
            if (!samCmd)
                throw std::runtime_error("SAMAV3Commands not available");

            std::cout << "[AUTH] Performing SAM Host Authentication..." << std::endl;

            auto key = std::make_shared<logicalaccess::DESFireKey>();
            key->setKeyType(logicalaccess::DF_KEY_AES);
            key->setData(logicalaccess::BufferHelper::fromHexString(
                "00000000000000000000000000000000"));

            try
            {
                samCmd->SAMAV2ISO7816Commands::authenticateHost(key, 0x00);
                std::cout << "[AUTH] OK" << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "[AUTH ERROR] " << e.what() << std::endl;
                throw;
            }

            runPKITestGenerate(samCmd, reader, tests);
            //runPKIImportTest(samCmd, reader, key, importTests);
            
            std::cout << "Logical automatic card removal in 3 seconds..." << std::endl;
            if (!readerConfig->getReaderUnit()->waitRemoval(3000))
                std::cerr << "Card removal forced." << std::endl;
            std::cout << "Card removed." << std::endl;
            readerConfig->getReaderUnit()->disconnect();
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << "ERROR : " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
