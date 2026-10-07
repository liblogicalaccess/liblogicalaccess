/**
 * \file basic_duox_aes_authentication.cpp
 * \brief Integration tests for DUOX AES-128 and AES-256 application authentication.
 *
 * Creates dedicated test applications on a DUOX card, verifies their configured key type
 * and authenticates using the corresponding application key.
 *
 * The test uses the PICC master key only for application lifecycle operations.
 * By default the card is erased before testing to ensure that deleted applications don't leave allocated NV memory behind.
 *
 * Only DUOX_TEST_APPLICATION_AID is created and deleted by the test.
 * The PICC master key and all application keys are left unchanged.
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/duoxcommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

/*
 * ==============================================
 * DUOX AES authentication test
 * ==============================================
 *
 * Purpose
 * -------
 * This test verifies :
 *
 *  1. PICC-level AES-128 authentication.
 *  2. Creation of a dedicated AES-128 application.
 *  3. AES-128 application authentication.
 *  4. Creation of a dedicated AES-256 application.
 *  5. AES-256 application authentication.
 *
 * The test does NOT exercise :
 *
 *  - ChangeKey()
 *  - ChangeKeyEV2()
 *  - ECC
 *  - CA Root Keys
 *  - ManageKeyPair()
 *  - Key sets
 *  - SetConfiguration()
 *  - RollKeySet()
 *  - InitializeKeySet()
 * 
 * Card preparation
 * ---------------
 * CARD_PREPARATION_MODE is deliberately set to EraseCard.
 *
 * Deleting an application does not necessarily release its allocated
 * NV-memory. The test therefore performs a full PICC erase before creating
 * the test applications in order to obtain a known, clean NV-memory baseline.
 *
 * WARNING :
 *  EraseCard is destructive and removes ALL applications and files from the card.
 *  This test must therefore only be executed against a dedicated test card.
 *
 * Safety model
 * ------------
 *  - The PICC Master Key is used only for authentication.
 *  - The PICC Master Key is never modified.
 *  - Two dedicated test applications are created :
 *        DUOX_TEST_AES128_APPLICATION_AID
 *        DUOX_TEST_AES256_APPLICATION_AID
 *  - Only these two applications are deleted by the test's RAII cleanup.
 *  - No ChangeKey operation is performed.
 *  - No existing application is intentionally preserved because the test deliberately starts with a full PICC erase.
 *
 * Keys
 * ----
 *  - PICC Master Key : AES-128
 *  - AES-128 application key 0 : AES-128
 *  - AES-256 application key 0 : AES-256
 *
 * ==============================================
 */

// ==============================================
// Configuration
// ==============================================

namespace
{

enum class DUOXCardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

constexpr DUOXCardPreparationMode CARD_PREPARATION_MODE = DUOXCardPreparationMode::EraseCard;

constexpr char READER_PROVIDER[] = "PCSC";
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000;

// PICC level
constexpr std::uint32_t PICC_LEVEL_AID = 0x000000;

// Dedicated test applications. These applications must never be provisioned for any other purpose
constexpr std::uint32_t DUOX_TEST_AES128_APPLICATION_AID = 0x123455;
constexpr std::uint32_t DUOX_TEST_AES256_APPLICATION_AID = 0x123456;

// PICC Master Key number
constexpr std::uint8_t PICC_MASTER_KEY_NO = 0;

// Application authentication key number
constexpr std::uint8_t TEST_APPLICATION_AUTH_KEY_NO = 0;

// GetKeySettings MaxNoOfKeys byte :
// bits 7..6 = AES key type
// 10b       = AES-128 (0x80)
// 11b       = AES-256 (0xC0)
// TODO Improve this later (should be placed somewhere else)
constexpr std::uint8_t AES128_KEY_TYPE = 0x80;
constexpr std::uint8_t AES256_KEY_TYPE = 0xC0;

// ==============================================
// Test keys
// ==============================================

const ByteVector TEST_AES128_AUTH_KEY_VALUE(16, 0x00);
const ByteVector TEST_AES256_AUTH_KEY_VALUE(32, 0x00);

// ==============================================
// Formatting / assertions
// ==============================================

std::string formatAid(const std::uint32_t aid)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << aid;
    return stream.str();
}

std::string formatHexByte(const std::uint8_t value)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(value);
    return stream.str();
}

void require(const bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}

template <typename T>
void requireNonNull(const T &pointer, const std::string &message)
{
    if (!pointer)
        throw std::runtime_error(message);
}

// ==============================================
// Keys
// ==============================================

std::shared_ptr<logicalaccess::DESFireKey> createTestPiccMasterKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    // Test PICC Master Key
    // This must match the actual AES PICC Master Key provisioned on the test card
    key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");

    return key;
}
std::shared_ptr<logicalaccess::DESFireKey> createTestAES128ApplicationKey()
{
    require(TEST_AES128_AUTH_KEY_VALUE.size() == 16,
        "Internal test error : AES-128 authentication key must contain exactly 16 bytes.");

    auto key = std::make_shared<logicalaccess::DESFireKey>();

    key->setKeyType(logicalaccess::DF_KEY_AES);
    key->fromString(logicalaccess::BufferHelper::getHex(TEST_AES128_AUTH_KEY_VALUE));

    require(key->getLength() == 16, "Internal test error : DESFireKey did not retain the expected AES-128 key length.");

    return key;
}

std::shared_ptr<logicalaccess::DESFireKey> createTestAES256ApplicationKey()
{
    require(TEST_AES256_AUTH_KEY_VALUE.size() == 32,
        "Internal test error : AES-256 authentication key must contain exactly 32 bytes.");

    auto key = std::make_shared<logicalaccess::DESFireKey>();

    key->setKeyType(logicalaccess::DF_KEY_AES256); // The actual AES-256 size is represented by the 32-byte key material
    key->fromString(logicalaccess::BufferHelper::getHex(TEST_AES256_AUTH_KEY_VALUE));
    require(key->getLength() == 32, "Internal test error : DESFireKey did not retain the expected AES-256 key length.");

    return key;
}

// ==============================================
// Reader handling
// ==============================================

std::shared_ptr<logicalaccess::ReaderUnit> findReaderByName(
    const std::shared_ptr<logicalaccess::ReaderProvider> &provider,
    const std::string &readerName)
{
    requireNonNull(provider, "PCSC reader provider is null.");

    const auto readers = provider->getReaderList();

    require(!readers.empty(), "No PCSC readers detected.");

    for (const auto &reader : readers)
    {
        if (reader && reader->getName() == readerName)
            return reader;
    }

    std::string availableReaders;

    for (const auto& reader : readers)
    {
        if (!reader)
            continue;
        if (!availableReaders.empty())
            availableReaders += "\n";
        availableReaders += "  - ";
        availableReaders += reader->getName();
    }

    throw std::runtime_error("Required PCSC reader was not found :\n  Expected : " +
                             readerName + "\nAvailable readers :\n" + availableReaders);
}

std::shared_ptr<logicalaccess::ReaderConfiguration> createReaderConfiguration()
{
    auto configuration = std::make_shared<logicalaccess::ReaderConfiguration>();
    auto *libraryManager = logicalaccess::LibraryManager::getInstance();

    requireNonNull(libraryManager, "LogicalAccess LibraryManager is unavailable.");

    const auto provider = libraryManager->getReaderProvider(READER_PROVIDER);

    requireNonNull(provider, "Unable to load the PCSC reader provider.");

    const auto reader = findReaderByName(provider, DESFIRE_READER_NAME);
    configuration->setReaderProvider(provider);
    configuration->setReaderUnit(reader);

    return configuration;
}

class ReaderConnectionGuard
{
  public:
    explicit ReaderConnectionGuard(
        const std::shared_ptr<logicalaccess::ReaderUnit> &reader)
        : reader_(reader)
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
            std::cerr << "[WARNING] Failed to disconnect reader during cleanup.\n";
        }
    }

    ReaderConnectionGuard(const ReaderConnectionGuard &)            = delete;
    ReaderConnectionGuard &operator=(const ReaderConnectionGuard &) = delete;

    ReaderConnectionGuard(ReaderConnectionGuard &&)            = delete;
    ReaderConnectionGuard &operator=(ReaderConnectionGuard &&) = delete;

  private:
    std::shared_ptr<logicalaccess::ReaderUnit> reader_;
};

std::shared_ptr<logicalaccess::Chip> waitForCard(const std::shared_ptr<logicalaccess::ReaderConfiguration> &configuration)
{
    requireNonNull(configuration, "Reader configuration is null.");

    const auto reader = configuration->getReaderUnit();

    requireNonNull(reader, "Configured reader is null.");

    std::cout << "[INFO] Reader : " << reader->getName() << '\n'
              << "[INFO] Waiting " << CARD_INSERTION_TIMEOUT_MS / 1000
              << " seconds for card insertion...\n";

    // Timestamp provides a consistent reference point when comparing different test executions
    const auto startTime = std::time(nullptr);

    if (startTime != static_cast<std::time_t>(-1))
        std::cout << "[INFO] Time start : " << std::ctime(&startTime);

    // Establish the connection with the physical reader
    reader->connectToReader();

    require(reader->waitInsertion(CARD_INSERTION_TIMEOUT_MS), "No card was inserted within the timeout.");
    require(reader->connect(), "A card was detected, but communication with the card could not be established.");

    const auto chip = reader->getSingleChip();

    requireNonNull(chip, "The reader connected successfully, but LogicalAccess could not identify the card.");

    return chip;
}

bool isDUOXCard(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    return chip && chip->getCardType() == "DUOX";
}

// ==============================================
// DUOX test context
// ==============================================

class DUOXTestContext
{
  public:
    explicit DUOXTestContext(const std::shared_ptr<logicalaccess::Chip> &chip)
        : chip_(chip)
    {
        requireNonNull(chip_, "DUOX chip is null.");

        desfireCommands_ = std::dynamic_pointer_cast<logicalaccess::DESFireCommands>(chip_->getCommands());
        requireNonNull(desfireCommands_, "DUOX card does not expose DESFireCommands.");

        duoxCommands_ = std::dynamic_pointer_cast<logicalaccess::DUOXCommands>(chip_->getCommands());
        requireNonNull(duoxCommands_, "DUOX card does not expose DUOXCommands.");

        desfireChip_ = std::dynamic_pointer_cast<logicalaccess::DESFireChip>(chip_);
        requireNonNull(desfireChip_, "DUOX card is not accessible as DESFireChip.");
    }

    const std::shared_ptr<logicalaccess::DESFireCommands> &desfire() const
    {
        return desfireCommands_;
    }

    const std::shared_ptr<logicalaccess::DUOXCommands> &duox() const
    {
        return duoxCommands_;
    }

    const std::shared_ptr<logicalaccess::DESFireChip> &desfireChip() const
    {
        return desfireChip_;
    }

    auto crypto() const
    {
        const auto crypto = desfireChip_->getCrypto();
        requireNonNull(crypto, "DESFire crypto context is not initialized.");
        return crypto;
    }

    void selectPicc()
    {
        std::cout << "[INFO] Selecting PICC level...\n";
        desfireCommands_->selectApplication(PICC_LEVEL_AID);
    }

    void selectApplication(const std::uint32_t aid)
    {
        require(aid != PICC_LEVEL_AID, "Safety failure : application AID cannot equal PICC level.");
        std::cout << "[INFO] Selecting application " << formatAid(aid) << "...\n";
        desfireCommands_->selectApplication(aid);
    }

    void selectAES128Application()
    {
        selectApplication(DUOX_TEST_AES128_APPLICATION_AID);
    }

    void selectAES256Application()
    {
        selectApplication(DUOX_TEST_AES256_APPLICATION_AID);
    }

    void authenticatePicc()
    {
        selectPicc();
        std::cout << "[INFO] Authenticating PICC Master Key " << "(AES-128)...\n";
        duoxCommands_->authenticateEV2First(PICC_MASTER_KEY_NO, createTestPiccMasterKey());
    }

    void authenticateApplication(const std::uint32_t aid, const std::shared_ptr<logicalaccess::DESFireKey> &key,
                                 const char *description)
    {
        selectApplication(aid);
        std::cout << "[INFO] Authenticating application " << formatAid(aid) << " using " << description << " key...\n";
        duoxCommands_->authenticateEV2First(TEST_APPLICATION_AUTH_KEY_NO, key);
        assertApplicationAuthentication(aid);
    }

    void assertApplicationAuthentication(const std::uint32_t expectedAid) const
    {
        const auto state = crypto();

        require(state->d_currentAid == expectedAid,
                "Authentication succeeded but the current AID is unexpected.");

        require(state->d_auth_method == logicalaccess::CryptoMethod::CM_EV2,
                "Authentication succeeded but the resulting authentication method is not EV2.");

        require(state->d_currentKeyNo == TEST_APPLICATION_AUTH_KEY_NO,
            "Authentication succeeded but the authenticated key number is unexpected.");
    }

  private:
    std::shared_ptr<logicalaccess::Chip> chip_;
    std::shared_ptr<logicalaccess::DESFireCommands> desfireCommands_;
    std::shared_ptr<logicalaccess::DUOXCommands> duoxCommands_;
    std::shared_ptr<logicalaccess::DESFireChip> desfireChip_;
};

// ==============================================
// Application helpers
// ==============================================

std::vector<unsigned int> listApplications(const std::shared_ptr<logicalaccess::DESFireCommands> &commands)
{
    requireNonNull(commands, "DESFire command interface is null.");

    const auto applications = commands->getApplicationIDs();

    std::cout << "\n========================================\n"
              << " Applications currently on card\n"
              << "========================================\n";

    if (applications.empty())
    {
        std::cout << "[INFO] No applications reported.\n";
    }
    else
    {
        for (const auto aid : applications)
            std::cout << "  - AID " << formatAid(aid) << '\n';
    }
    std::cout << "========================================\n";
    return applications;
}

bool applicationExists(const std::vector<unsigned int> &applications, const std::uint32_t aid)
{
    return std::find(applications.begin(), applications.end(), aid) != applications.end();
}

void requireApplicationsAbsent(DUOXTestContext &context)
{
    const auto applications = context.desfire()->getApplicationIDs();

    require(!applicationExists(applications, DUOX_TEST_AES128_APPLICATION_AID),
            "AES-128 test application still exists after card preparation.");

    require(!applicationExists(applications, DUOX_TEST_AES256_APPLICATION_AID),
            "AES-256 test application still exists after card preparation.");
}

// ==============================================
// Card preparation
// ==============================================

void deleteApplication(DUOXTestContext &context, const std::uint32_t aid)
{
    context.authenticatePicc();

    const auto applications = context.desfire()->getApplicationIDs();

    if (!applicationExists(applications, aid))
    {
        std::cout << "[INFO] Application " << formatAid(aid) << " does not exist.\n";
        return;
    }

    std::cout << "[INFO] Deleting test application " << formatAid(aid) << "...\n";

    context.desfire()->deleteApplication(aid);

    const auto remaining = context.desfire()->getApplicationIDs();

    require(!applicationExists(remaining, aid), "Test application still exists after deletion.");

    std::cout << "[PASS] Test application " << formatAid(aid) << " deleted.\n";
}

void eraseTestCard(DUOXTestContext &context)
{
    std::cout << "\n========================================\n"
              << " DUOX TEST CARD ERASE\n"
              << "========================================\n"
              << "[WARNING] This operation will permanently remove ALL applications and files from the card.\n"
              << "[WARNING] The PICC Master Key will NOT be modified.\n";

    context.authenticatePicc();

    std::cout << "[INFO] Erasing / formatting PICC...\n";

    // IMPORTANT :
    // Erase is a PICC-level destructive operation !
    // Do not rely on the previous authentication/session remaining valid after the erase
    context.desfire()->erase();

    std::cout << "[PASS] PICC erased successfully.\n"
              << "[INFO] All applications and files were removed.\n";

    const auto applications = context.desfire()->getApplicationIDs();

    require(applications.empty(), "PICC erase completed but applications are still reported.");

    std::cout << "[PASS] Application list is empty after erase.\n"
              << "[INFO] NV memory is now available for reuse.\n"
              << "=============================================\n";
}

void prepareDUOXTestCard(DUOXTestContext &context)
{
    std::cout << "\n========================================\n"
              << " DUOX CARD PREPARATION\n"
              << "========================================\n";

    // 1 - Show current applications before test preparation
    listApplications(context.desfire());

    // 2 - Format the card to release all previously allocated NV memory
    // OR
    // If dedicated test application already exists, deletes it
    if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
    {
        std::cout << "[INFO] Preparation mode : FULL PICC ERASE\n";
        eraseTestCard(context);
    }
    else
    {
        std::cout << "[INFO] Preparation mode : DELETE TEST APPLICATION ONLY\n";
        deleteApplication(context, DUOX_TEST_AES128_APPLICATION_AID);
        deleteApplication(context, DUOX_TEST_AES256_APPLICATION_AID);
    }

    requireApplicationsAbsent(context);
    std::cout << "[PASS] Test application AIDs are available.\n"
              << "===========================================\n";
}

// ==============================================
// Test application RAII cleanup
// ==============================================

class DUOXTestApplicationGuard
{
  public:
    DUOXTestApplicationGuard(DUOXTestContext &context, const std::uint32_t aid)
        : context_(context)
        , aid_(aid)
        , active_(true)
    {
        require(aid_ != PICC_LEVEL_AID, "Test application guard cannot target PICC level.");
    }

    ~DUOXTestApplicationGuard()
    {
        if (!active_)
            return;

        try
        {
            cleanup();
        }
        catch (const std::exception &exception)
        {
            std::cerr << "[WARNING] Failed to automatically delete test application " << formatAid(aid_)
                      << " during automatic cleanup : " << exception.what() << '\n'
                      << "[WARNING] Manual cleanup may be required.\n";
        }
        catch (...)
        {
            std::cerr << "[WARNING] Failed to automatically delete test application " << formatAid(aid_)
                      << " during automatic cleanup due to an unknown error.\n"
                      << "[WARNING] Manual cleanup may be required.\n";
        }
    }

    DUOXTestApplicationGuard(const DUOXTestApplicationGuard &)            = delete;
    DUOXTestApplicationGuard &operator=(const DUOXTestApplicationGuard &) = delete;

    DUOXTestApplicationGuard(DUOXTestApplicationGuard &&)            = delete;
    DUOXTestApplicationGuard &operator=(DUOXTestApplicationGuard &&) = delete;

    void cleanup()
    {
        if (!active_)
            return;

        deleteApplication(context_, aid_);

        active_ = false;
    }

  private:
    DUOXTestContext &context_;
    std::uint32_t aid_;
    bool active_;
};

// ==============================================
// Safety / diagnostics
// ==============================================

void printTestSafetyPolicy()
{
    std::cout << "\n================= DUOX TEST SAFETY POLICY =================\n"
              << "[SAFETY] Preparation mode    : " << (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard
                      ? "FULL PICC ERASE" : "DELETE TEST APPLICATIONS") << '\n'
              << "[SAFETY] AES-128 test AID    : " << formatAid(DUOX_TEST_AES128_APPLICATION_AID) << '\n'
              << "[SAFETY] AES-256 test AID    : " << formatAid(DUOX_TEST_AES256_APPLICATION_AID) << '\n'
              << "[SAFETY] PICC Master Key     : AUTHENTICATION ONLY\n"
              << "[SAFETY] ChangeKey           : NOT USED\n"
              << "[SAFETY] ChangeKeyEV2        : NOT USED\n"
              << "[SAFETY] ECC / CA operations : NOT USED\n"
              << "[SAFETY] Key-set operations  : NOT USED\n"
              << "[SAFETY] SetConfiguration    : NOT USED\n"
              << "[SAFETY] ManageKeyPair       : NOT USED\n";

    if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
    {
        std::cout << "[SAFETY] CARD CONTENT        : WILL BE ERASED\n"
                  << "[SAFETY] NV memory           : CLEAN BASELINE\n";
    }
    else
    {
        std::cout << "[SAFETY] CARD CONTENT        : TEST APPLICATIONS ONLY\n"
                  << "[SAFETY] NV memory           : NOT RESET\n";
    }

    std::cout << "===========================================================\n";
}

// ==============================================
// Application creation
// ==============================================

void createAES128TestApplication(DUOXTestContext &context)
{
    std::cout << "\n[INFO] Creating dedicated AES-128 test application "
              << formatAid(DUOX_TEST_AES128_APPLICATION_AID) << "...\n";

    context.authenticatePicc();

    context.duox()->createApplication(DUOX_TEST_AES128_APPLICATION_AID,
                                      logicalaccess::KS_DEFAULT, 1,
                                      logicalaccess::DF_KEY_AES);

    std::cout << "[PASS] Dedicated AES-128 application created.\n";
}

void createAES256TestApplication(DUOXTestContext &context)
{
    std::cout << "\n[INFO] Creating dedicated AES-256 test application "
              << formatAid(DUOX_TEST_AES256_APPLICATION_AID) << "...\n";

    context.authenticatePicc();

    context.duox()->createApplication(DUOX_TEST_AES256_APPLICATION_AID,
                                      logicalaccess::KS_DEFAULT, 1,
                                      logicalaccess::DF_KEY_AES256);

    std::cout << "[PASS] Dedicated AES-256 application created.\n";
}

// ==============================================
// Application configuration verification
// ==============================================

void verifyApplicationConfiguration(DUOXTestContext &context, const std::uint32_t aid,
                                    const std::uint8_t expectedKeyType, const char *description)
{
    std::cout << "\n[TEST] Verify " << description << " application configuration...\n";

    context.selectApplication(aid);

    const auto settings = context.duox()->getKeySettings();

    require(!settings.piccLevel, "GetKeySettings unexpectedly reports PICC-level context.");

    require(settings.numberOfKeys == 1,
        std::string("Newly created ") + description + " test application does not contain exactly one application key.");

    require(settings.keyType == expectedKeyType,
            std::string("Newly created ") + description +
                " test application has an unexpected key type. Expected " +
                formatHexByte(expectedKeyType) + ", received " +
                formatHexByte(settings.keyType) + ".");

    std::cout << "[PASS] " << description << " application reports exactly one " << description << " key.\n"
              << "[INFO] Number of keys : " << static_cast<unsigned int>(settings.numberOfKeys) << '\n'
              << "[INFO] Key type       : " << formatHexByte(settings.keyType) << '\n';
}

// ==============================================
// Authentication tests
// ==============================================

// Authentication uses application key 0, which was created by CreateApplication()

// Authentication succeeded if authenticateEV2First ()  returned without throwing.
// Verify the resulting LogicalAccess crypto state as well.

void testPiccAES128Authentication(DUOXTestContext &context)
{
    std::cout << "\n========================================\n"
              << " TEST : PICC AES-128 authentication\n"
              << "========================================\n";

    context.authenticatePicc();

    const auto crypto = context.crypto();

    require(crypto->d_currentAid == PICC_LEVEL_AID,
            "PICC authentication succeeded but current AID is not PICC level.");

    require(crypto->d_currentKeyNo == PICC_MASTER_KEY_NO,
            "PICC authentication succeeded but authenticated key number is unexpected.");

    require(crypto->d_auth_method == logicalaccess::CryptoMethod::CM_EV2,
            "PICC authentication succeeded but authentication method is not EV2.");

    std::cout << "[PASS] PICC AES-128 EV2 authentication succeeded.\n";
}

void testAES128ApplicationAuthentication(DUOXTestContext &context)
{
    std::cout << "\n==========================================\n"
              << " TEST : AES-128 application authentication\n"
              << "==========================================\n";

    const auto authenticationKey = createTestAES128ApplicationKey();

    require(authenticationKey->getKeyType() == logicalaccess::DF_KEY_AES, "Test authentication key is not AES-128.");

    require(authenticationKey->getLength() == 16, "Test authentication key is not 16 bytes / AES-128.");

    context.authenticateApplication(DUOX_TEST_AES128_APPLICATION_AID, authenticationKey, "AES-128");

    std::cout << "[PASS] AES-128 application authentication succeeded.\n"
              << "[PASS] Current AID remains " << formatAid(DUOX_TEST_AES128_APPLICATION_AID) << ".\n"
              << "[PASS] Authenticated key number is " << static_cast<unsigned int>(TEST_APPLICATION_AUTH_KEY_NO) << ".\n"
              << "[PASS] Authentication method is EV2.\n";
}

void testAES256ApplicationAuthentication(DUOXTestContext &context)
{
    std::cout << "\n==========================================\n"
              << " TEST : AES-256 application authentication\n"
              << "==========================================\n";

    const auto authenticationKey = createTestAES256ApplicationKey();

    require(authenticationKey->getKeyType() == logicalaccess::DF_KEY_AES256, "Test authentication key is not AES-256.");

    require(authenticationKey->getLength() == 32, "Test authentication key is not 32 bytes / AES-256.");

    context.authenticateApplication(DUOX_TEST_AES256_APPLICATION_AID, authenticationKey, "AES-256");

    std::cout << "[PASS] AES-256 application authentication succeeded.\n"
              << "[PASS] Current AID remains " << formatAid(DUOX_TEST_AES256_APPLICATION_AID) << ".\n"
              << "[PASS] Authenticated key number is " << static_cast<unsigned int>(TEST_APPLICATION_AUTH_KEY_NO) << ".\n"
              << "[PASS] Authentication method is EV2.\n";
}

// ==============================================
// Complete test
// ==============================================

void runDUOXAESAuthenticationTest(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    DUOXTestContext context(chip);

    std::cout << "\n==============================================\n"
              << " DUOX AES AUTHENTICATION TEST\n"
              << "==============================================\n";

    printTestSafetyPolicy();

    // The card is erased first because application deletion doesn't release all allocated NV memory on the target card.
    prepareDUOXTestCard(context);

    // Explicitly verify that both dedicated AIDs are available before creating anything.
    requireApplicationsAbsent(context);

    // -------------------------------------------------------------------------
    // PICC AES-128 authentication
    // -------------------------------------------------------------------------

    testPiccAES128Authentication(context);

    // -------------------------------------------------------------------------
    // AES-128 application
    // -------------------------------------------------------------------------

    std::cout << "\n[STEP 1] Creating AES-128 application...\n";

    createAES128TestApplication(context);

    DUOXTestApplicationGuard aes128Cleanup(context, DUOX_TEST_AES128_APPLICATION_AID);

    verifyApplicationConfiguration(context, DUOX_TEST_AES128_APPLICATION_AID, AES128_KEY_TYPE, "AES-128");

    testAES128ApplicationAuthentication(context);

    // -------------------------------------------------------------------------
    // AES-256 application
    // -------------------------------------------------------------------------

    std::cout << "\n[STEP 2] Creating AES-256 application...\n";

    createAES256TestApplication(context);

    DUOXTestApplicationGuard aes256Cleanup(context, DUOX_TEST_AES256_APPLICATION_AID);

    verifyApplicationConfiguration(context, DUOX_TEST_AES256_APPLICATION_AID, AES256_KEY_TYPE, "AES-256");

    testAES256ApplicationAuthentication(context);

    // Both application guards delete their respective applications when leaving this scope.
    // Full erase on next test restores baseline NV-memory.
    std::cout << "\n==============================================\n"
              << " AES-128 / AES-256 AUTHENTICATION TEST PASSED\n"
              << "==============================================\n";
}

} // namespace

// ==============================================
// Main
// ==============================================

int main()
{
    /*
     * SAFETY RULES
     *
     * 1 - CARD_PREPARATION_MODE defaults to EraseCard.
     * 2 - EraseCard is intentionally destructive and removes all applications and files from the dedicated test card.
     * 3 - The PICC Master Key is used only for authentication.
     * 4 - The PICC Master Key is never modified.
     * 5 - Two dedicated test applications are created :
     *         DUOX_TEST_AES128_APPLICATION_AID
     *         DUOX_TEST_AES256_APPLICATION_AID
     * 6 - Only those two test applications are deleted by RAII cleanup.
     * 7 - No ChangeKey operation is performed.
     * 8 - No ECC or CA operation is performed.
     * 9 - No key-set operation is performed.
     * 10 - No other application is modified.
     * 11 - The dedicated test applications are deleted by RAII.
     */

    try
    {
        std::cout << "\n===========================================================\n"
                  << " DUOX AES AUTHENTICATION TEST\n"
                  << "===========================================================\n"
                  << "[INFO] AES-128 test application : " << formatAid(DUOX_TEST_AES128_APPLICATION_AID) << '\n'
                  << "[INFO] AES-256 test application : " << formatAid(DUOX_TEST_AES256_APPLICATION_AID) << '\n'
                  << "[INFO] PICC authentication      : AES-128 / EV2\n"
                  << "[INFO] AES-128 app authentication: AES-128 / EV2\n"
                  << "[INFO] AES-256 app authentication: AES-256 / EV2\n"
                  << "===========================================================\n";

        if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
        {
            std::cout << "[WARNING] CARD_PREPARATION_MODE = EraseCard\n"
                << "[WARNING] ALL applications and files will be deleted.\n"
                << "[WARNING] This test must only be run on a dedicated test card.\n";
        }
        else
        {
            std::cout << "[INFO] CARD_PREPARATION_MODE = DeleteTestApplications\n"
                      << "[INFO] Only the dedicated test applications may be deleted.\n";
        }

        const auto readerConfiguration = createReaderConfiguration();
        const auto reader = readerConfiguration->getReaderUnit();
        requireNonNull(reader, "Reader disappeared after configuration.");

        ReaderConnectionGuard readerGuard(reader);

        const auto chip = waitForCard(readerConfiguration);

        std::cout << "[INFO] Card inserted on reader \"" << reader->getConnectedName() << "\".\n";

        const auto uid = reader->getNumber(chip);

        std::cout << "[INFO] Card type : " << chip->getCardType() << '\n'
                  << "[INFO] Card UID  : " << logicalaccess::BufferHelper::getHex(uid) << '\n';

        if (!isDUOXCard(chip))
        {
            std::cout << "[INFO] Card is not a DUOX card. Nothing will be modified.\n";
            return EXIT_SUCCESS;
        }

        std::cout << "[INFO] Card is a DUOX card.\n";

        // Execute the actual test. Application guard owns cleanup after successful creation.
        runDUOXAESAuthenticationTest(chip);

        std::cout << "[INFO] Tests completed successfully.\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "[ERROR] " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "[ERROR] Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
}