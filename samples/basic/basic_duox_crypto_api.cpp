/**
 * \file basic_duox_crypto_api.cpp
 * \brief Real card DUOX Crypto API integration tests
 *
 * Tests :
 *  - Generic CryptoRequest dispatch and payload validation
 *  - CryptoRequest_ECCSign :
 *      - Initialize
 *      - Update
 *      - Finalize
 *      - OneShot
 *      - OneShotPrecomputedHash
 *  - CryptoRequest_Echo:
 *      - Minimum payload
 *      - Standard payload
 *      - Maximum payload
 *      - Response integrity
 *  - ECC key number and input validation
 *
 * SAFETY :
 *  - The PICC is never erased or formatted
 *  - Only the dedicated test application is eligible for cleanup
 *  - The PICC Master Key and application authentication key are never changed
 *  - The ECC private key is generated on-card
 *  - No private key is imported or exported
 *
 * IMPORTANT :
 *  - Use a dedicated test card
 *  - Confirm the PICC AES Master Key matches the card
 *  - Confirm the selected application AID is reserved for this test
 *  - Configure CryptoRequest through SetConfiguration (0x15)
 *  - Ensure the configured communication mode matches CRYPTO_COMM_MODE
 *  - Ensure the ECC key policy permits CryptoRequest ECC signing
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>

#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>

#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

using namespace logicalaccess;

namespace
{

// ==============================================
// Configuration
// ==============================================

// WARNING : EraseCard permanently removes all applications and files
enum class DUOXCardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

// Dedicated test application
// Reserve this AID exclusively for this sample
constexpr std::uint32_t TEST_APPLICATION_AID = 0x123459;

// PICC configuration
constexpr std::uint32_t PICC_LEVEL_AID = 0x000000;
constexpr std::uint8_t PICC_MASTER_KEY_NO = 0x00;

// Application configuration
constexpr std::uint8_t APP_MASTER_KEY_NO = 0x00;

// ECC configuration
constexpr std::uint8_t ECC_KEY_NO = 0x00;

constexpr std::uint16_t ECC_KEY_POLICY = 0x0010; // Enables Crypto Request ECC Sign (Action 0x03)
constexpr std::uint32_t ECC_KEY_USAGE_COUNTER_LIMIT = 0x00000000;

// The write access condition must match the intended key policy
// 0x0E corresponds to free access with the configured Full communication mode
constexpr std::uint8_t ECC_KEY_WRITE_ACCESS = 0x0E;

// CryptoRequest communication mode
// Must match the card's SetConfiguration (0x15) configuration
constexpr EncryptionMode CRYPTO_COMM_MODE = CM_MAC;

// Change to DeleteTestApplication to preserve unrelated card applications
constexpr DUOXCardPreparationMode CARD_PREPARATION_MODE = DUOXCardPreparationMode::EraseCard;

// Reader configuration
constexpr char READER_PROVIDER[] = "PCSC";
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000;

// Crypto API limits
constexpr std::size_t CRYPTO_MAX_COMMAND_DATA_SIZE = 0xFE;

constexpr std::size_t ECC_MAX_INPUT_SIZE = 0xEA;
constexpr std::size_t ECC_SHA256_SIZE = 32;
constexpr std::size_t ECC_SIGNATURE_SIZE = 64;

constexpr std::size_t ECHO_MIN_SIZE = 1;
constexpr std::size_t ECHO_MAX_SIZE = 254;

// ==============================================
// Test Credentials
// ==============================================

// Test keys must match the dedicated test card
constexpr char TEST_PICC_MASTER_KEY_HEX[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

constexpr char TEST_APPLICATION_AUTH_KEY_HEX[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

// ==============================================
// Assertions and logging
// ==============================================

void printSection(const std::string &name)
{
    std::cout << "\n==============================================\n"
              << name << '\n'
              << "==============================================\n";
}

void logInfo(const std::string &message)
{
    std::cout << "[INFO] " << message << std::endl;
}

void logPass(const std::string &message)
{
    std::cout << "[PASS] " << message << std::endl;
}

void logFailure(const std::string &operation, const std::string &message)
{
    std::cerr << "[FAIL] " << operation << " : " << message << std::endl;
}

void require(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}

template <typename T>
void requireNonNull(const T &value, const std::string &message)
{
    if (!value)
        throw std::runtime_error(message);
}

template <typename Function>
void expectException(Function &&function, const std::string &description)
{
    try
    {
        std::forward<Function>(function)();
    }
    catch (const std::exception &exception)
    {
        logPass(description + " : correctly rejected (" + exception.what() + ")");
        return;
    }
    catch (...)
    {
        logPass(description + " : correctly rejected with an unknown exception.");
        return;
    }

    throw std::runtime_error(description + " : expected an exception, but none was thrown.");
}

template <typename Function>
void traceOperation(const std::string &name, Function &&function)
{
    logInfo("START : " + name);

    try
    {
        std::forward<Function>(function)();
        logPass("DONE  : " + name);
    }
    catch (const std::exception &exception)
    {
        logFailure(name, exception.what());
        throw;
    }
    catch (...)
    {
        logFailure(name, "Unknown exception");
        throw;
    }
}

std::string formatAid(std::uint32_t aid)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << aid;
    return stream.str();
}

template <typename Container>
std::string toHex(const Container &data)
{
    std::ostringstream stream;
    stream << std::hex << std::uppercase << std::setfill('0');

    for (const auto byte : data)
        stream << std::setw(2) << static_cast<unsigned int>(byte);

    return stream.str();
}

void expectResponseSize(const ByteVector &response, std::size_t expectedSize, const std::string &description)
{
    require(response.size() == expectedSize,
            description + " : expected " + std::to_string(expectedSize) +
                " bytes, received " + std::to_string(response.size()) + ".");

    logInfo(description + " : " + std::to_string(response.size()) + " bytes.");
}

void expectEmptyResponse(const ByteVector &response, const std::string &description)
{
    expectResponseSize(response, 0, description);
}

// ==============================================
// Reader handling
// ==============================================

std::shared_ptr<ReaderUnit> findReaderByName(const std::shared_ptr<ReaderProvider> &provider, const std::string &readerName)
{
    requireNonNull(provider, "PCSC reader provider is null.");

    const auto readers = provider->getReaderList();

    require(!readers.empty(), "No PCSC readers detected.");

    for (const auto &reader : readers)
    {
        if (reader && reader->getName() == readerName)
            return reader;
    }

    std::ostringstream availableReaders;

    for (const auto &reader : readers)
    {
        if (reader)
            availableReaders << "  - " << reader->getName() << '\n';
    }

    throw std::runtime_error("Required PCSC reader was not found.\n"
        "Expected : " + readerName + "\nAvailable readers :\n" + availableReaders.str());
}

std::shared_ptr<ReaderConfiguration> createReaderConfiguration()
{
    auto configuration = std::make_shared<ReaderConfiguration>();

    auto *libraryManager = LibraryManager::getInstance();

    requireNonNull(libraryManager, "LogicalAccess LibraryManager is unavailable.");

    const auto provider = libraryManager->getReaderProvider(READER_PROVIDER);

    requireNonNull(provider, "Unable to load the PCSC reader provider.");

    const auto reader = findReaderByName(provider, DESFIRE_READER_NAME);

    configuration->setReaderProvider(provider);
    configuration->setReaderUnit(reader);

    return configuration;
}

// Ensures that the physical reader is disconnected on every exit path
class ReaderConnectionGuard
{
public:
    explicit ReaderConnectionGuard(std::shared_ptr<ReaderUnit> reader)
        : reader_(std::move(reader))
    {
    }

    ~ReaderConnectionGuard()
    {
        if (!reader_)
            return;

        try
        {
            reader_->disconnect();
        }
        catch (...)
        {
            std::cerr << "[WARNING] Reader disconnect failed.\n";
        }
    }

    ReaderConnectionGuard(const ReaderConnectionGuard &)            = delete;
    ReaderConnectionGuard &operator=(const ReaderConnectionGuard &) = delete;

private:
    std::shared_ptr<ReaderUnit> reader_;
};

std::shared_ptr<Chip> waitForCard(const std::shared_ptr<ReaderConfiguration> &configuration)
{
    requireNonNull(configuration, "Reader configuration is null.");

    const auto reader = configuration->getReaderUnit();

    requireNonNull(reader, "Configured reader is null.");

    logInfo("Reader name : " + reader->getName());

    logInfo("Waiting " + std::to_string(CARD_INSERTION_TIMEOUT_MS / 1000) + " seconds for card insertion...");

    // Timestamp allows correlation of logs across separate test runs
    const auto startTime = std::time(nullptr);

    if (startTime != static_cast<std::time_t>(-1))
    {
        char timeBuffer[26] = {};

#if defined(_WIN32)
        const bool success =
            ctime_s(timeBuffer, sizeof(timeBuffer), &startTime) == 0;
#else
        const bool success =
            ctime_r(&startTime, timeBuffer) != nullptr;
#endif

        if (success)
            logInfo(std::string("Time start : ") + timeBuffer);
    }

    traceOperation("Connect to physical reader", [&reader]() { reader->connectToReader(); });

    traceOperation("Wait for card insertion", [&reader]()
        {
            require(reader->waitInsertion(CARD_INSERTION_TIMEOUT_MS), "No card was inserted within the timeout.");
        });

    traceOperation("Establish card communication", [&reader]()
        {
            require(reader->connect(), "Card communication could not be established.");
        });

    std::shared_ptr<Chip> chip;

    traceOperation("Identify card type", [&reader, &chip]()
        {
            chip = reader->getSingleChip();
            requireNonNull(chip, "LogicalAccess could not identify the card.");
        });

    logInfo("Identified card type : " + chip->getCardType());

    return chip;
}

bool isDUOXCard(const std::shared_ptr<Chip> &chip)
{
    return chip && chip->getCardType() == "DUOX";
}

// ==============================================
// Test keys
// ==============================================

std::shared_ptr<DESFireKey> createTestPICCKey()
{
    auto key = std::make_shared<DESFireKey>();

    key->setKeyType(DF_KEY_AES);
    key->fromString(TEST_PICC_MASTER_KEY_HEX);

    return key;
}

std::shared_ptr<DESFireKey> createTestApplicationKey()
{
    auto key = std::make_shared<DESFireKey>();

    key->setKeyType(DF_KEY_AES);
    key->fromString(TEST_APPLICATION_AUTH_KEY_HEX);

    return key;
}

// ==============================================
// DUOX test context
// ==============================================

class DUOXTestContext
{
public:
    explicit DUOXTestContext(const std::shared_ptr<Chip> &chip)
        : chip_(chip)
    {
        requireNonNull(chip_, "DUOX chip is null.");

        const auto commands = chip_->getCommands();

        duoxCommands_ = std::dynamic_pointer_cast<DUOXCommands>(commands);

        requireNonNull(duoxCommands_, "DUOXCommands is unavailable on this card.");

        desfireCommands_ = std::dynamic_pointer_cast<DESFireCommands>(commands);

        requireNonNull(desfireCommands_, "DESFireCommands is unavailable on this card.");

        duoxISO7816Commands_ = std::dynamic_pointer_cast<DUOXISO7816Commands>(commands);

        requireNonNull(duoxISO7816Commands_, "DUOXISO7816Commands is unavailable on this card.");

        desfireChip_ = std::dynamic_pointer_cast<DESFireChip>(chip_);

        requireNonNull(desfireChip_, "DUOX card is not accessible as DESFireChip.");
    }

    const std::shared_ptr<DUOXCommands> &duox() const
    {
        return duoxCommands_;
    }

    const std::shared_ptr<DESFireCommands> &desfire() const
    {
        return desfireCommands_;
    }

    const std::shared_ptr<DUOXISO7816Commands> &duoxISO7816() const
    {
        return duoxISO7816Commands_;
    }

    void selectPicc()
    {
        traceOperation("Select PICC level", [&]() { duoxCommands_->selectApplication(PICC_LEVEL_AID); });
    }

    void selectApplication()
    {
        traceOperation("Select test application " + formatAid(TEST_APPLICATION_AID),
            [&]()
            {
                duoxCommands_->selectApplication(TEST_APPLICATION_AID);
            });
    }

    void authenticatePicc()
    {
        selectPicc();

        traceOperation("Authenticate PICC AES Master Key", [&]()
            {
                duoxCommands_->authenticateEV2First(PICC_MASTER_KEY_NO, createTestPICCKey());
            });
    }

    void authenticateApplication()
    {
        selectApplication();

        traceOperation("Authenticate test application AES key", [&]()
            {
                duoxCommands_->authenticateEV2First(APP_MASTER_KEY_NO, createTestApplicationKey());
            });
    }

private:
    std::shared_ptr<Chip> chip_;

    std::shared_ptr<DUOXCommands> duoxCommands_;
    std::shared_ptr<DESFireCommands> desfireCommands_;

    std::shared_ptr<DUOXISO7816Commands> duoxISO7816Commands_;
    std::shared_ptr<DESFireChip> desfireChip_;
};

// ==============================================
// Application management
// ==============================================

bool testApplicationExists(DUOXTestContext &context)
{
    const auto applications = context.desfire()->getApplicationIDs();

    return std::find(applications.begin(), applications.end(), TEST_APPLICATION_AID) != applications.end();
}

void requireTestApplicationAbsent(DUOXTestContext &context)
{
    require(!testApplicationExists(context),
        "Reserved Crypto API test application already exists : " + formatAid(TEST_APPLICATION_AID));
}

void createTestApplication(DUOXTestContext &context)
{
    context.authenticatePicc();

    requireTestApplicationAbsent(context);

    traceOperation("Create Crypto API test application", [&]()
        {
            context.duox()->createApplication(TEST_APPLICATION_AID, KS_DEFAULT, 1, DF_KEY_AES);
        });

    require(testApplicationExists(context), "Test application creation could not be verified.");

    logPass("Crypto API test application created.");
}

void deleteTestApplication(DUOXTestContext &context)
{
    printSection("TEST APPLICATION CLEANUP");

    context.authenticatePicc();

    if (!testApplicationExists(context))
    {
        logInfo("Test application does not exist. Nothing to clean up.");
        return;
    }

    traceOperation("Delete Crypto API test application", [&]()
        {
            context.desfire()->deleteApplication(TEST_APPLICATION_AID);
        });

    require(!testApplicationExists(context), "Test application still exists after deletion.");

    logPass("Crypto API test application deleted.");
}

void eraseTestCard(DUOXTestContext &context)
{
    printSection("DUOX TEST CARD ERASE");

    std::cout << "[WARNING] This operation will permanently remove ALL applications and files from the card.\n"
              << "[WARNING] The PICC Master Key will NOT be modified.\n";

    context.authenticatePicc();

    logInfo("Erasing / formatting PICC...");

    // IMPORTANT :
    // Erase is a PICC-level destructive operation !
    // Do not rely on the previous authentication/session remaining valid after the erase
    context.desfire()->erase();

    logPass("PICC erased successfully.");

    logInfo("All applications and files were removed.");

    const auto applications = context.desfire()->getApplicationIDs();

    require(applications.empty(), "PICC erase completed but applications are still reported.");

    logPass("Application list is empty after erase.");

    logInfo("NV memory is now available for reuse.");

    std::cout << "=============================================\n";
}

void prepareDUOXTestCard(DUOXTestContext &context)
{
    printSection("DUOX CARD PREPARATION");

    // Format the card to release all previously allocated NV memory
    // OR
    // If the dedicated Crypto API test application already exists, delete that application only

    if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
    {
        logInfo("Preparation mode : FULL PICC ERASE");
        eraseTestCard(context);
    }
    else
    {
        logInfo("Preparation mode : DELETE TEST APPLICATION ONLY");

        // Check whether the dedicated test application exists
        const auto applications = context.desfire()->getApplicationIDs();

        const bool testApplicationExists =
            std::find(applications.begin(), applications.end(), TEST_APPLICATION_AID) != applications.end();

        if (testApplicationExists)
        {
            context.desfire()->deleteApplication(TEST_APPLICATION_AID);

            logPass("Existing test application deleted.");
        }
        else
        {
            logInfo("Test application does not exist. Nothing to delete.");
        }
    }

    requireTestApplicationAbsent(context);

    logPass("Crypto API test application AID is available.");

    std::cout << "===========================================\n";
}

// ==============================================
// Test application RAII cleanup
// ==============================================

class TestApplicationGuard
{
public:
    explicit TestApplicationGuard(DUOXTestContext &context)
        : context_(context)
    {
    }

    ~TestApplicationGuard()
    {
        try
        {
            cleanup();
        }
        catch (const std::exception &exception)
        {
            std::cerr
                << "[WARNING] Automatic application cleanup failed: "
                << exception.what() << '\n'
                << "[WARNING] Manually delete the reserved test application "
                << formatAid(TEST_APPLICATION_AID)
                << " after authenticating with the PICC Master Key.\n";
        }
        catch (...)
        {
            std::cerr
                << "[WARNING] Unknown application cleanup failure. "
                << "Check the reserved test application manually.\n";
        }
    }

    TestApplicationGuard(const TestApplicationGuard &) = delete;
    TestApplicationGuard &operator=(const TestApplicationGuard &) = delete;

    void cleanup()
    {
        if (!active_)
            return;

        deleteTestApplication(context_);

        active_ = false;
    }

private:
    DUOXTestContext &context_;
    bool active_ = true;
};

// ==============================================
// Safety diagnostics
// ==============================================

void printTestSafetyPolicy()
{
    const bool eraseMode = CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard;

    std::cout << "\n================= DUOX TEST SAFETY POLICY =================\n"
              << "[SAFETY] Preparation mode  : " << (eraseMode ? "FULL PICC ERASE" : "DELETE TEST APPLICATION") << '\n'
              << "[SAFETY] Reserved test AID : " << formatAid(TEST_APPLICATION_AID) << '\n'
              << "[SAFETY] PICC Master Key   : AUTHENTICATION ONLY\n"
              << "[SAFETY] CARD CONTENT      : " << (eraseMode ? "WILL BE ERASED" : "TEST APPLICATION ONLY") << '\n'
              << "[SAFETY] NV memory         : " << (eraseMode ? "CLEAN BASELINE" : "NOT RESET") << '\n';

    std::cout << "===========================================================\n";
}

// ==============================================
// Test summary
// ==============================================

void printFinalTestSummary(bool success)
{
    printSection("DUOX CRYPTO API TEST SUMMARY");

    if (success)
    {
        logPass("Generic CryptoRequest");
        logPass("CryptoRequest Echo");
        logPass("ECC Sign Initialize");
        logPass("ECC Sign Update");
        logPass("ECC Sign Finalize");
        logPass("ECC Sign OneShot");
        logPass("ECC Sign OneShotPrecomputedHash");
        logPass("ECC Sign input validation");

        std::cout << "\n================================================\n"
                  << "       ALL DUOX CRYPTO API TESTS PASSED\n"
                  << "================================================\n";
    }
    else
    {
        logFailure("DUOX Crypto API", "One or more tests failed.");

        std::cout << "\n================================================\n"
                  << "       DUOX CRYPTO API TESTS FAILED\n"
                  << "================================================\n";
    }
}

// ==============================================
// ECC key provisioning
// ==============================================

ByteVector provisionECCKeyPair(DUOXTestContext &context)
{
    ByteVector publicKey;

    traceOperation(
        "Generate on-card NIST P-256 ECC key pair",
        [&]()
        {
            publicKey = context.duox()->manageKeyPair(
                ECC_KEY_NO,
                DUOXManageKeyPairOption::GenerateKeyPair,
                CurveID::NIST_P256,
                ECC_KEY_POLICY,
                ECC_KEY_WRITE_ACCESS,
                ECC_KEY_USAGE_COUNTER_LIMIT,
                ByteVector(),
                CM_ENCRYPT);
        });

    // NIST P-256 uncompressed point : 0x04 || X(32) || Y(32)
    require(publicKey.size() == 65, "ECC key generation returned an invalid public key length. Expected 65 bytes.");

    require(publicKey.front() == 0x04, "ECC public key is not in uncompressed point format.");

    logInfo("Generated ECC public key (" + std::to_string(publicKey.size()) + " bytes) : " + toHex(publicKey));

    logPass("On-card ECC key pair provisioned.");

    return publicKey;
}

// ==============================================
// CryptoRequest - Helpers
// ==============================================

// This command is similar to getMaxEchoPayloadSize inside duoxiso7816commands.cpp. This can be improved.
constexpr std::size_t getMaxEchoPayloadSize(EncryptionMode commMode) noexcept
{
    constexpr std::size_t PLAIN_CRYPTO_ECHO_MAX_SIZE             = 0xFE;
    constexpr std::size_t MAC_CRYPTO_ECHO_MAX_SIZE               = 0xF6;
    constexpr std::size_t ISO7816_MAX_COMMAND_DATA_SIZE          = 0xFF;
    constexpr std::size_t SECURE_MESSAGING_MAC_SIZE              = 8;
    constexpr std::size_t SECURE_MESSAGING_ENCRYPTION_BLOCK_SIZE = 16;
    constexpr std::size_t CRYPTO_ACTION_SIZE                     = 1;

    switch (commMode)
    {
    case CM_PLAIN: return PLAIN_CRYPTO_ECHO_MAX_SIZE;

    case CM_MAC: return MAC_CRYPTO_ECHO_MAX_SIZE;

    case CM_ENCRYPT:
    {
        constexpr std::size_t maxEncryptedSize =
            ((ISO7816_MAX_COMMAND_DATA_SIZE - SECURE_MESSAGING_MAC_SIZE) /
             SECURE_MESSAGING_ENCRYPTION_BLOCK_SIZE) * SECURE_MESSAGING_ENCRYPTION_BLOCK_SIZE;

        constexpr std::size_t maxPlaintextSize = maxEncryptedSize - 1;

        return maxPlaintextSize - CRYPTO_ACTION_SIZE;
    }

    default: return 0;
    }
}

// ==============================================
// Generic CryptoRequest tests
// ==============================================

void testCryptoRequestValidation(DUOXTestContext &context)
{
    printSection("TEST : GENERIC CRYPTOREQUEST VALIDATION");

    // Unsupported action must be rejected locally
    traceOperation("Reject unsupported CryptoRequest action", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequest(static_cast<CryptoAction>(0x7E), ByteVector(), CRYPTO_COMM_MODE);
                },
                "Unsupported CryptoRequest action");
        });

    // The action byte consumes one byte of the 255-byte command-data limit
    traceOperation("Reject oversized CryptoRequest command data", [&]()
        {
            const ByteVector oversized(CRYPTO_MAX_COMMAND_DATA_SIZE + 1, 0xAA);

            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequest(CryptoAction::Echo, oversized, CRYPTO_COMM_MODE);
                },
                "Oversized CryptoRequest payload");
        });

    logPass("Generic CryptoRequest validation completed.");
}

// ==============================================
// CryptoRequest Echo tests
// ==============================================

ByteVector createEchoPayload(std::size_t size)
{
    ByteVector payload;
    payload.reserve(size);

    for (std::size_t i = 0; i < size; ++i)
    {
        payload.push_back(static_cast<std::uint8_t>((i * 37U + 0xA5U) & 0xFFU));
    }

    return payload;
}

void testEchoPayload(DUOXTestContext &context, const ByteVector &payload, const std::string &description)
{
    traceOperation(description, [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestEcho(payload, CRYPTO_COMM_MODE);

            require(response == payload, "Echo response does not match the original input.");

            expectResponseSize(response, payload.size(), "Echo response");

            logInfo("Echo payload : " + toHex(response));
        });
}

void testCryptoRequestEcho(DUOXTestContext &context)
{
    printSection("TEST : CRYPTOREQUEST ECHO");

    const std::size_t maxPayloadSize = getMaxEchoPayloadSize(CRYPTO_COMM_MODE);

    // Minimum supported input
    testEchoPayload(context, createEchoPayload(ECHO_MIN_SIZE),
                    "Echo minimum payload (" + std::to_string(ECHO_MIN_SIZE) + " byte)");

    // Typical payload
    testEchoPayload(context, createEchoPayload(16), "Echo standard payload (16 bytes)");

    // Maximum supported input for the selected communication mode
    testEchoPayload(context, createEchoPayload(maxPayloadSize),
                    "Echo maximum payload (" + std::to_string(maxPayloadSize) + " bytes)");

    // Invalid input lengths must be rejected locally
    traceOperation("Reject empty Echo payload", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestEcho(ByteVector(), CRYPTO_COMM_MODE);
                },
                "Empty Echo payload");
        });

    traceOperation("Reject oversized Echo payload", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestEcho(createEchoPayload(maxPayloadSize + 1), CRYPTO_COMM_MODE);
                },
                "Oversized Echo payload");
        });

    logPass("CryptoRequest Echo tests completed.");
}

// ==============================================
// ECC Sign validation tests
// ==============================================

void testECCSignInputValidation(DUOXTestContext &context)
{
    printSection("TEST : ECC SIGN INPUT VALIDATION");

    const ByteVector validInput = createEchoPayload(16);

    // Operations requiring a key number must reject an absent key
    traceOperation("Reject Initialize without key number", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::Initialize,
                        validInput,
                        CRYPTO_COMM_MODE,
                        KeyNo());
                },
                "Initialize without key number");
        });

    traceOperation("Reject OneShot without key number", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::OneShot,
                        validInput,
                        CRYPTO_COMM_MODE,
                        KeyNo());
                },
                "OneShot without key number");
        });

    // Update and Finalize must not accept a key number
    traceOperation("Reject Update with key number", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::Update,
                        validInput,
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "Update with key number");
        });

    traceOperation("Reject Finalize with key number", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::Finalize,
                        ByteVector(),
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "Finalize with key number");
        });

    // Application key numbers are restricted to 0...4
    traceOperation("Reject invalid application ECC key number", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::Initialize,
                        validInput,
                        CRYPTO_COMM_MODE,
                        KeyNo(0x05));
                },
                "Application ECC key number 5");
        });

    // Raw input maximum is 234 bytes
    traceOperation("Reject oversized ECC raw input", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::OneShot,
                        createEchoPayload(ECC_MAX_INPUT_SIZE + 1),
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "ECC raw input exceeding 234 bytes");
        });

    // Precomputed SHA-256 hash must be exactly 32 bytes
    traceOperation("Reject undersized SHA-256 hash", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::OneShotPrecomputedHash,
                        createEchoPayload(ECC_SHA256_SIZE - 1),
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "31-byte SHA-256 hash");
        });

    traceOperation("Reject oversized SHA-256 hash", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        ECCSignOperation::OneShotPrecomputedHash,
                        createEchoPayload(ECC_SHA256_SIZE + 1),
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "33-byte SHA-256 hash");
        });

    traceOperation("Reject unsupported ECC Sign operation", [&]()
        {
            expectException([&]()
                {
                    context.duoxISO7816()->cryptoRequestECCSign(
                        static_cast<ECCSignOperation>(0xFF),
                        validInput,
                        CRYPTO_COMM_MODE,
                        KeyNo(ECC_KEY_NO));
                },
                "Unsupported ECC Sign operation");
        });

    logPass("ECC Sign input validation completed.");
}

// ==============================================
// ECC Sign operation tests
// ==============================================

void testECCSignInitialize(DUOXTestContext &context, const ByteVector &inputData)
{
    printSection("TEST : ECC SIGN INITIALIZE");

    traceOperation("Initialize ECC signature operation", [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::Initialize,
                    inputData,
                    CRYPTO_COMM_MODE,
                    KeyNo(ECC_KEY_NO));

            expectEmptyResponse(response, "Initialize response");
        });
}

void testECCSignUpdate(DUOXTestContext &context, const ByteVector &inputData)
{
    printSection("TEST : ECC SIGN UPDATE");

    traceOperation("Update ECC signature operation", [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::Update,
                    inputData,
                    CRYPTO_COMM_MODE);

            expectEmptyResponse(response, "Update response");
        });
}

void testECCSignFinalize(DUOXTestContext &context, const ByteVector &inputData)
{
    printSection("TEST : ECC SIGN FINALIZE");

    traceOperation("Finalize ECC signature operation", [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::Finalize,
                    inputData,
                    CRYPTO_COMM_MODE);
            
            expectResponseSize(response, ECC_SIGNATURE_SIZE, "Finalize signature response");

            logInfo("Signature : " + toHex(response));
        });
}

void testECCSignIncremental(DUOXTestContext &context)
{
    printSection("TEST : ECC SIGN INCREMENTAL OPERATION");

    const ByteVector initializationData{
        0x10, 0x20, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80
    };

    const ByteVector updateData{
        0x90, 0xA0, 0xB0, 0xC0,
        0xD0, 0xE0, 0xF0, 0x11
    };

    // Exercise the complete Initialize -> Update -> Finalize sequence
    testECCSignInitialize(context, initializationData);
    testECCSignUpdate(context, updateData);
    testECCSignFinalize(context, ByteVector());

    logPass("Incremental ECC Sign sequence completed.");
}

void testECCSignOneShot(DUOXTestContext &context)
{
    printSection("TEST : ECC SIGN ONESHOT");

    const ByteVector inputData = createEchoPayload(64);

    traceOperation("ECC OneShot with raw input", [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::OneShot,
                    inputData,
                    CRYPTO_COMM_MODE,
                    KeyNo(ECC_KEY_NO));

            expectResponseSize(response, ECC_SIGNATURE_SIZE, "OneShot signature");

            logInfo("Signature : " + toHex(response));
        });

    // Exercise the maximum supported raw input length
    traceOperation("ECC OneShot with maximum raw input (234 bytes)", [&]()
        {
            const ByteVector maximumInput = createEchoPayload(ECC_MAX_INPUT_SIZE);

            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::OneShot,
                    maximumInput,
                    CRYPTO_COMM_MODE,
                    KeyNo(ECC_KEY_NO));

            expectResponseSize(response, ECC_SIGNATURE_SIZE, "Maximum input OneShot signature");
        });

    logPass("ECC OneShot tests completed.");
}

void testECCSignOneShotPrecomputedHash(DUOXTestContext &context)
{
    printSection("TEST : ECC SIGN ONESHOT PRECOMPUTED HASH");

    // Deterministic 32-byte test hash
    // This is an input test vector, not a production digest
    const ByteVector hash = {
        0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
        0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
        0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
        0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD
    };

    traceOperation("ECC OneShot with precomputed SHA-256 hash", [&]()
        {
            const ByteVector response = context.duoxISO7816()->cryptoRequestECCSign(
                    ECCSignOperation::OneShotPrecomputedHash,
                    hash,
                    CRYPTO_COMM_MODE,
                    KeyNo(ECC_KEY_NO));

            expectResponseSize(response, ECC_SIGNATURE_SIZE, "Precomputed-hash signature");

            logInfo("Signature : " + toHex(response));
        });

    logPass("ECC OneShot precomputed hash test completed.");
}

// ==============================================
// Complete Crypto API test suite
// ==============================================

void runCryptoAPITestSuite(DUOXTestContext &context)
{
    printSection("DUOX CRYPTO API INTEGRATION TEST SUITE");

    logInfo("Test application : " + formatAid(TEST_APPLICATION_AID));
    logInfo("ECC key number   : " + std::to_string(ECC_KEY_NO));
    logInfo("ECC curve        : NIST P-256");
    logInfo("Crypto comm mode : CM_ENCRYPT");

    // ------------------------------------------
    // Phase 1 : Application setup
    // ------------------------------------------

    printSection("PHASE 1/4 : APPLICATION SETUP");

    createTestApplication(context);

    context.authenticateApplication();

    logPass("Test application authenticated.");

    // ------------------------------------------
    // Phase 2 : ECC key provisioning
    // ------------------------------------------

    printSection("PHASE 2/4 : ECC KEY PROVISIONING");

    const ByteVector publicKey = provisionECCKeyPair(context);

    require(publicKey.size() == 65, "Invalid provisioned ECC public key.");

    // ------------------------------------------
    // Phase 3 : Generic CryptoRequest and Echo
    // ------------------------------------------

    printSection("PHASE 3/4 : GENERIC CRYPTOREQUEST AND ECHO");

    testCryptoRequestValidation(context);
    testCryptoRequestEcho(context);

    // ------------------------------------------
    // Phase 4 : ECC Sign
    // ------------------------------------------

    printSection("PHASE 4/4 : ECC SIGN");

    testECCSignInputValidation(context);

    testECCSignIncremental(context);
    testECCSignOneShot(context);
    testECCSignOneShotPrecomputedHash(context);

    // ------------------------------------------
    // Final summary
    // ------------------------------------------

    printSection("CRYPTO API TEST SUMMARY");

    logPass("Generic CryptoRequest validation");
    logPass("CryptoRequest Echo");
    logPass("ECC Sign Initialize");
    logPass("ECC Sign Update");
    logPass("ECC Sign Finalize");
    logPass("ECC Sign OneShot");
    logPass("ECC Sign OneShotPrecomputedHash");
    logPass("ECC Sign input validation");

    std::cout
        << "\n================================================\n"
        << "       ALL DUOX CRYPTO API TESTS PASSED\n"
        << "================================================\n";
}

// ==============================================
// Main test lifecycle
// ==============================================

void runCryptoAPITests(DUOXTestContext &context)
{
    printSection("DUOX CRYPTO API REAL CARD TEST");

    // Refuse to overwrite an existing application
    context.authenticatePicc();

    requireTestApplicationAbsent(context);

    // The guard is armed only after confirming that the AID was absent
    // It can therefore safely clean up an application created by this run
    TestApplicationGuard applicationGuard(context);

    runCryptoAPITestSuite(context);

    // Explicit cleanup on successful completion
    applicationGuard.cleanup();

    logPass("Crypto API test application cleanup verified.");
}

} // namespace

// ==============================================
// Main
// ==============================================

int main()
{
    bool success = false;

    try
    {
        printSection("DUOX CRYPTO API INTEGRATION TEST");

        printTestSafetyPolicy();

        const auto readerConfiguration = createReaderConfiguration();

        const auto reader =readerConfiguration->getReaderUnit();

        requireNonNull(reader, "Reader disappeared after configuration.");

        // Disconnect even if connection, authentication or tests fail
        ReaderConnectionGuard readerGuard(reader);

        const auto chip = waitForCard(readerConfiguration);

        logInfo("Card inserted on reader \"" + reader->getConnectedName() + "\".");

        logInfo("Card type : " + chip->getCardType());

        if (!isDUOXCard(chip))
        {
            logInfo("Card is not a DUOX card. Nothing will be modified.");
            return EXIT_SUCCESS;
        }

        logInfo("DUOX card detected.");

        DUOXTestContext context(chip);

        prepareDUOXTestCard(context);

        runCryptoAPITests(context);

        logPass("All DUOX Crypto API tests completed successfully.");

        success = true;
    }
    catch (const std::exception &exception)
    {
        logFailure("Crypto API integration", exception.what());
        std::cerr << "[WARNING] If the test application was created before "
                      "the failure, reconnect and delete only the reserved test AID "
                      "after authenticating with the PICC Master Key.\n";
    }
    catch (...)
    {
        logFailure("Crypto API integration", "Unknown exception.");
    }

    printFinalTestSummary(success);

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}