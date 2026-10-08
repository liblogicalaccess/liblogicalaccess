#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

/*
 * This executable serves both as a functional test suite and as a usage example for DUOX commands
 * The operations exercised here are representative of real application workflows
 * while the test-specific safety restrictions keep all destructive operations isolated to the dedicated test application
 *
 * Safety model :
 *   - Only DUOX_TEST_APPLICATION_AID may be created/deleted
 *   - PICC-level authentication is used only for test-application lifecycle
 *   - PICC Master Key is never modified
 *   - ECC operations are restricted to the dedicated test application
 *   - ChangeKey tests may modify only application key 1
 *   - The test application is automatically removed after the test run
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

// Select the physical reader by its stable PC/SC name instead of enumeration index
// This prevents reader ordering changes from selecting the wrong device
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000;

// PICC / application
constexpr std::uint32_t PICC_LEVEL_AID = 0x000000;
// Dedicated test application. This application must never be provisioned for any other
// purpose
constexpr std::uint32_t DUOX_TEST_APPLICATION_AID = 0x123456;

// Authentication keys
// Authenticate using PICC master key number 0
constexpr std::uint8_t PICC_MASTER_KEY_NO = 0;
// Authenticate using key number 0 of DUOX_TEST_APPLICATION_AID
constexpr std::uint8_t TEST_APPLICATION_AUTH_KEY_NO = 0;

// ECC key slots
// ECC private key slot inside the dedicated test application
constexpr std::uint8_t TEST_ECC_KEY_SLOT_P256      = 0x00; // DUOX ECC key slot 0
constexpr std::uint8_t TEST_ECC_KEY_SLOT_BRAINPOOL = 0x01;
constexpr std::uint8_t TEST_ECC_KEY_SLOT_IMPORT    = 0x02;
constexpr std::uint8_t TEST_ECC_KEY_SLOT_METADATA  = 0x03;

// ECC configuration
constexpr std::uint32_t DUOX_TEST_KUC_LIMIT  = 0;      // No key-usage limit
constexpr std::uint16_t DUOX_TEST_KEY_POLICY = 0x0000; // No ECC operation enabled

constexpr auto DUOX_COMM_MODE = logicalaccess::DUOXCommunicationMode::Full;

constexpr std::uint8_t DUOX_TEST_WRITE_AR  = 0x0E; // Free access
constexpr std::uint8_t DUOX_COMM_MODE_BITS = static_cast<std::uint8_t>(DUOX_COMM_MODE) << 4;
constexpr std::uint8_t DUOX_TEST_WRITE_ACCESS = DUOX_TEST_WRITE_AR | DUOX_COMM_MODE_BITS; // Free access

// CA Root Key
// CA Root Key slot inside the dedicated test application
constexpr std::uint8_t TEST_CA_ROOT_KEY_SLOT = 0;
// No CA Root Key access rights enabled by default
constexpr std::uint16_t DUOX_TEST_CA_ACCESS_RIGHTS = 0x0000;
// CA Root Key WriteAccess
constexpr std::uint8_t DUOX_TEST_CA_WRITE_ACCESS = DUOX_TEST_WRITE_AR | DUOX_COMM_MODE_BITS;
// CA Root Key ReadAccess
constexpr std::uint8_t DUOX_TEST_CA_READ_ACCESS = DUOX_TEST_WRITE_AR | DUOX_COMM_MODE_BITS;
// Certificate revocation disabled
constexpr std::uint8_t DUOX_TEST_CA_CRL_FILE      = 0x00;
constexpr std::uint32_t DUOX_TEST_CA_CRL_FILE_AID = 0x000000;
// A recognizable test issuer
const ByteVector DUOX_TEST_CA_ISSUER{'D', 'U', 'O', 'X', '-', 'T', 'E', 'S', 'T'};

// GetKeySettings
constexpr auto GET_KEY_SETTINGS_DEFAULT = logicalaccess::DUOXKeySettingsOption::KeySettings;
constexpr auto GET_KEY_SETTINGS_ECC_PRIVATE_KEY = logicalaccess::DUOXKeySettingsOption::ECCPrivateKeyMetadata;
constexpr auto GET_KEY_SETTINGS_CA_ROOT_KEY = logicalaccess::DUOXKeySettingsOption::CARootKeyMetadata;

constexpr std::size_t EXPECTED_P256_PUBLIC_KEY_SIZE   = 65U;
constexpr std::size_t EXPECTED_ECC_METADATA_COUNT     = 4U;
constexpr std::size_t EXPECTED_CA_ROOT_METADATA_COUNT = 1U;

// GetKeySettings MaxNoOfKeys byte :
// bits 7..6 = AES key type
// 10b = AES-128 (0x80)
// 11b = AES-256 (0xC0)
// TODO Improve this later (should be placed somewhere else)
constexpr std::uint8_t KEY_TYPE_AES128 = 0x80;
constexpr std::uint8_t KEY_TYPE_AES256 = 0xC0;

// ChangeKey
constexpr std::uint8_t SAFE_CHANGE_KEY_NO = 1;

// These are deliberately different from the real authentication key
const ByteVector CHANGE_KEY_VALUE_A{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

const ByteVector CHANGE_KEY_VALUE_B{0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                                    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};

const ByteVector CHANGE_KEY_VALUE_C{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};

// ==============================================
// Formatting / assertions
// ==============================================

std::string formatAid(std::uint32_t aid)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << aid;
    return stream.str();
}

void require(bool condition, const std::string &message)
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

std::shared_ptr<logicalaccess::DESFireKey> createTestApplicationAuthKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    // Actual key provisioned in the dedicated test application (only 0 by default)
    key->fromString("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    return key;
}

std::shared_ptr<logicalaccess::DESFireKey> makeTestAESKey(const ByteVector &value, std::uint8_t version)
{
    require(value.size() == 16, "Internal test error : AES test key must be 16 bytes.");

    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    key->fromString(logicalaccess::BufferHelper::getHex(value));
    key->setKeyVersion(version);

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

std::shared_ptr<logicalaccess::Chip> waitForCard(const std::shared_ptr<logicalaccess::ReaderConfiguration> &configuration)
{
    requireNonNull(configuration, "Reader configuration is null.");

    const auto reader = configuration->getReaderUnit();

    requireNonNull(reader, "Configured reader is null.");

    std::cout << "[INFO] Reader: " << reader->getName() << '\n'
              << "[INFO] Waiting " << CARD_INSERTION_TIMEOUT_MS / 1000
              << " seconds for card insertion...\n";

    // The timestamp is not particularly useful by itself but it provides a consistent reference point
    // when comparing different test executions
    const auto startTime = std::time(nullptr);

    if (startTime != static_cast<std::time_t>(-1))
        std::cout << "[INFO] Time start : " << std::ctime(&startTime);

    // Establish the connection with the physical reader
    reader->connectToReader();

    // LogicalAccess is allowed to identify the inserted card normally
    if (!reader->waitInsertion(CARD_INSERTION_TIMEOUT_MS))
    {
        reader->disconnect();
        throw std::runtime_error("No card was inserted within the timeout.");
    }
    if (!reader->connect())
    {
        reader->disconnect();
        throw std::runtime_error("A card was detected, but communication with the card could not be established.");
    }

    const auto chip = reader->getSingleChip();
    if (!chip)
    {
        reader->disconnect();
        throw std::runtime_error("The reader connected successfully, but LogicalAccess could not identify the card.");
    }

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

    const std::shared_ptr<logicalaccess::Chip> &chip() const
    {
        return chip_;
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

    void selectTestApplication()
    {
        assertTestAidIsNotPiccLevel();
        desfireCommands_->selectApplication(DUOX_TEST_APPLICATION_AID);
    }

    void authenticatePicc()
    {
        selectPicc();
        std::cout << "[INFO] Authenticating PICC Master Key...\n";
        duoxCommands_->authenticateEV2First(PICC_MASTER_KEY_NO, createTestPiccMasterKey());
    }

    void authenticateTestApplication()
    {
        selectTestApplication();
        duoxCommands_->authenticateEV2First(TEST_APPLICATION_AUTH_KEY_NO, createTestApplicationAuthKey());
        assertTestApplicationContext();
    }

    void assertTestApplicationContext() const
    {
        const auto state = crypto();
        require(state->d_currentAid == DUOX_TEST_APPLICATION_AID,
                "SAFETY FAILURE : current AID is not the dedicated DUOX test application.");
        require(state->d_auth_method == logicalaccess::CryptoMethod::CM_DUOX,
                "Safety check failed : active authentication is not DUOX.");
        require(state->d_currentKeyNo == TEST_APPLICATION_AUTH_KEY_NO,
                "Safety check failed : application key 0 is not authenticated.");
    }

    void assertChangeKeyContext(std::uint8_t keyNo) const
    {
        const auto state = crypto();
        require(state->d_currentAid == DUOX_TEST_APPLICATION_AID,
                "SAFETY FAILURE : ChangeKey is outside the dedicated test application.");
        require(logicalaccess::usesEV2Crypto(state->d_auth_method),
            "ChangeKey safety check : active authentication does not use EV2 crypto.");
        require(state->d_currentKeyNo == TEST_APPLICATION_AUTH_KEY_NO,
                "SAFETY FAILURE : ChangeKey requires application key 0.");
        require(keyNo != PICC_MASTER_KEY_NO,
            "SAFETY FAILURE : PICC Master Key may never be modified.");
        require(keyNo != TEST_APPLICATION_AUTH_KEY_NO,
            "SAFETY FAILURE : application authentication key may never be modified.");
        require(keyNo == SAFE_CHANGE_KEY_NO,
            "SAFETY FAILURE : only dedicated application key 1 may be modified.");
    }

    void restoreTestAuthentication()
    {
        authenticateTestApplication();
    }

  private:
    /*
     * Guardrail against accidentally treating the PICC level as the dedicated test
     * application This does not verify the currently selected AID : callers must validate
     * the card context before performing application-level operations
     */
    static void assertTestAidIsNotPiccLevel()
    {
        if (DUOX_TEST_APPLICATION_AID == PICC_LEVEL_AID)
            throw std::runtime_error("Safety check failed : DUOX test application AID cannot equal PICC level.");
    }

    std::shared_ptr<logicalaccess::Chip> chip_;
    std::shared_ptr<logicalaccess::DESFireCommands> desfireCommands_;
    std::shared_ptr<logicalaccess::DUOXCommands> duoxCommands_;
    std::shared_ptr<logicalaccess::DESFireChip> desfireChip_;
};

// ==============================================
// Application lifecycle
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

bool applicationExists(const std::vector<unsigned int> &applications, unsigned int aid)
{
    return std::find(applications.begin(), applications.end(), aid) != applications.end();
}

void deleteTestApplication(DUOXTestContext &context)
{
    std::cout << "[INFO] Selecting PICC level for test-application cleanup...\n";

    context.authenticatePicc();

    std::cout << "[INFO] Deleting ONLY test application " << formatAid(DUOX_TEST_APPLICATION_AID) << "...\n";

    context.desfire()->deleteApplication(DUOX_TEST_APPLICATION_AID);

    std::cout << "[PASS] Test application deleted.\n";
}

// Full erase is used by the default preparation mode because deleting the test application alone does not provide
// the same clean NV-memory baseline required by these memory lifecycle tests
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

    std::cout << "[PASS] PICC erased successfully.\n";
    std::cout << "[INFO] All applications and files were removed.\n";

    const auto applications = context.desfire()->getApplicationIDs();

    require(applications.empty(), "PICC erase completed but applications are still reported.");

    std::cout << "[PASS] Application list is empty after erase.\n"
              << "[INFO] NV memory is now available for reuse.\n"
              << "========================================\n";
}

void prepareDUOXTestCard(DUOXTestContext &context)
{
    std::cout << "\n========================================\n"
              << " DUOX CARD PREPARATION\n"
              << "========================================\n";

    // 1 - Show current applications before test preparation
    const auto applications = listApplications(context.desfire());

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

        if (applicationExists(applications, DUOX_TEST_APPLICATION_AID))
        {
            deleteTestApplication(context);
        }
        else
        {
            std::cout << "[INFO] Test application does not exist.\n";
        }
    }

    const auto remaining = listApplications(context.desfire());
    if (remaining.empty())
        std::cout << "[PASS] No applications remain after card preparation.\n";

    std::cout << "========================================\n";
}

// ==============================================
// Test application RAII cleanup
// ==============================================

class DUOXTestApplicationGuard
{
  public:
    explicit DUOXTestApplicationGuard(DUOXTestContext &context)
        : context_(context)
    {
        require(DUOX_TEST_APPLICATION_AID != PICC_LEVEL_AID, "DUOX test application guard cannot target PICC level.");

        // The guard is constructed only after CreateApplication succeeds
        // From this point on cleanup owns responsibility for the test application
        active_ = true;
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
            std::cerr << "[WARNING] Failed to automatically delete DUOX test application "
                      << formatAid(DUOX_TEST_APPLICATION_AID) << " during automatic cleanup : " << exception.what()
                      << '\n' << "[WARNING] Manual cleanup may be required.\n";
        }
        catch (...)
        {
            std::cerr << "[WARNING] Failed to automatically delete DUOX test application "
                      << formatAid(DUOX_TEST_APPLICATION_AID)
                      << " during automatic cleanup due to an unknown error.\n"
                      << "[WARNING] Manual cleanup may be required.\n";
        }
    }

    DUOXTestApplicationGuard(const DUOXTestApplicationGuard &)            = delete;
    DUOXTestApplicationGuard &operator=(const DUOXTestApplicationGuard &) = delete;

    void cleanup()
    {
        if (!active_)
            return;

        context_.authenticatePicc();
        const auto applications = context_.desfire()->getApplicationIDs();

        if (!applicationExists(applications, DUOX_TEST_APPLICATION_AID))
        {
            active_ = false;
            std::cout << "[CLEANUP] Test application no longer exists.\n";
            return;
        }

        std::cout << "[CLEANUP] Deleting ONLY DUOX test application " << formatAid(DUOX_TEST_APPLICATION_AID) << "...\n";

        context_.desfire()->deleteApplication(DUOX_TEST_APPLICATION_AID);
        const auto remaining = context_.desfire()->getApplicationIDs();

        require(!applicationExists(remaining, DUOX_TEST_APPLICATION_AID),
                "DUOX test application still exists after deletion.");

        active_ = false;

        std::cout << "[CLEANUP] DUOX test application deleted.\n";
    }

  private:
    DUOXTestContext &context_;
    bool active_ = false;
};

// ==============================================
// Safety / diagnostics
// ==============================================

/*
 * The test suite permits destructive operations ONLY on DUOX_TEST_APPLICATION_AID
 *
 * No other application and no PICC data are intentionally modified,
 * except the PICC-level authentication required to create/delete the dedicated test application
 */
void printTestSafetyPolicy()
{
    std::cout << "\n================= DUOX TEST SAFETY POLICY =================\n"
              << "[SAFETY] Test AID                      : " << formatAid(DUOX_TEST_APPLICATION_AID) << '\n'
              << "[SAFETY] PICC operations               : RESTRICTED TO TEST LIFECYCLE\n"
              << "[SAFETY] PICC Master Key               : AUTHENTICATION ONLY\n"
              << "[SAFETY] Application creation/deletion : ONLY " << formatAid(DUOX_TEST_APPLICATION_AID) << "\n"
              << "[SAFETY] ChangeKey                     : APPLICATION KEY 1 ONLY\n"
              << "[SAFETY] SetConfiguration              : DISABLED\n"
              << "[SAFETY] Key-set operations            : DISABLED\n"
              << "[SAFETY] Other application AIDs        : NEVER TOUCHED\n"
              << "[SAFETY] ManageKeyPair                 : ALLOWED ONLY after explicit test-AID authentication\n"
              << "============================================================\n";
}

// ==============================================
// Test helpers
// ==============================================

void requireAESKeySettings(const logicalaccess::DUOXKeySettings &settings,
                           std::uint8_t expectedKeyCount, bool expectedPiccLevel)
{
    require(settings.responseType == logicalaccess::DUOXKeySettingsOption::KeySettings,
            "DUOX GetKeySettings returned an unexpected response type.");
    require(settings.piccLevel == expectedPiccLevel,
            "DUOX GetKeySettings returned an unexpected PICC-level state.");
    require(settings.numberOfKeys == expectedKeyCount,
            "DUOX GetKeySettings returned an unexpected number of keys : "
            "expected " + std::to_string(expectedKeyCount) + " , received " + std::to_string(settings.numberOfKeys) + ".");
    require(settings.keyType == KEY_TYPE_AES128 || settings.keyType == KEY_TYPE_AES256,
            "DUOX GetKeySettings returned an invalid AES key type.");
    require(!settings.hasApplicationKeySetSettings,
            "DUOX GetKeySettings unexpectedly returned application key-set settings.");
}

void requireEV2TestAuthentication(DUOXTestContext &context, std::uint8_t keyNo = TEST_APPLICATION_AUTH_KEY_NO)
{
    const auto crypto = context.crypto();
    require(crypto->d_currentAid == DUOX_TEST_APPLICATION_AID, "SAFETY FAILURE: unexpected current AID.");
    require(crypto->d_currentKeyNo == keyNo, "Unexpected authenticated key number.");
    require(logicalaccess::usesEV2Crypto(crypto->d_auth_method), "Expected an authentication method using EV2 crypto.");
}

void authenticateWithKey(DUOXTestContext &context, std::uint8_t keyNo,
                         const std::shared_ptr<logicalaccess::DESFireKey> &key)
{
    context.duox()->authenticateEV2First(keyNo, key);
    requireEV2TestAuthentication(context, keyNo);
}

void changeAndVerifyKey(DUOXTestContext &context, std::uint8_t keyNo, std::uint8_t keySetNo,
                        const std::shared_ptr<logicalaccess::DESFireKey> &key, bool useEV2Command)
{
    context.assertChangeKeyContext(keyNo);

    if (useEV2Command)
        context.duox()->changeKeyEV2(keySetNo, keyNo, key);
    else
        context.duox()->changeKey(keyNo, key);

    authenticateWithKey(context, keyNo, key);

    context.restoreTestAuthentication();
}

// ==============================================
// ManageKeyPair tests
// ==============================================

// -------------------------------------------------------------------------
// TEST 0 - AuthenticateEV2NonFirst using the crypto key store fallback
// -------------------------------------------------------------------------
void testAuthenticateEV2NonFirst(DUOXTestContext &context)
{
    constexpr std::uint8_t TEST_KEY_SLOT = 0;
    constexpr std::uint8_t TEST_KEY_NO   = TEST_APPLICATION_AUTH_KEY_NO;

    std::cout << "\n[TEST 0] Verify AuthenticateEV2NonFirst key store fallback...\n";

    const auto crypto = context.crypto();

    const auto storedKey = crypto->getKey(TEST_KEY_SLOT, TEST_KEY_NO);

    requireNonNull(storedKey, "AuthenticateEV2NonFirst test failed : "
                       "DESFireCrypto::getKey() returned null for the test application's authentication key.");

    require(storedKey->getKeyType() == logicalaccess::DF_KEY_AES,
            "AuthenticateEV2NonFirst test failed : key returned by DESFireCrypto has the wrong key type.");

    std::cout << "[PASS] DESFireCrypto contains the expected authentication key.\n";

    // Do NOT pass testAuthKey here : this is specifically testing the fallback path that
    // retrieves the authentication key from DESFireCrypto
    context.duox()->authenticateEV2NonFirst(TEST_KEY_NO);

    std::cout << "[PASS] AuthenticateEV2NonFirst succeeded using the crypto key store fallback.\n";

    // AuthenticateEV2NonFirst must not change the selected application
    const auto cryptoAfterAuth = context.crypto();

    if (!cryptoAfterAuth)
        throw std::runtime_error("AuthenticateEV2NonFirst test failed : crypto context became null after authentication.");

    if (cryptoAfterAuth->d_currentAid != DUOX_TEST_APPLICATION_AID)
        throw std::runtime_error("SAFETY FAILURE : AuthenticateEV2NonFirst changed the current application.");

    std::cout << "[PASS] Current AID remains the dedicated DUOX test application.\n";
}

// -------------------------------------------------------------------------
// TEST 1 - Generate NIST P-256
// -------------------------------------------------------------------------
void testGenerateP256(DUOXTestContext &context)
{
    std::cout << "\n[TEST 1] Generate NIST P-256 key pair...\n";

    const auto publicKey = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_P256, logicalaccess::DUOXManageKeyPairOption::GenerateKeyPair,
        logicalaccess::CurveID::NIST_P256, DUOX_TEST_KEY_POLICY, DUOX_TEST_WRITE_ACCESS,
        DUOX_TEST_KUC_LIMIT);

    require(!publicKey.empty(),
            "DUOX NIST P-256 generation returned an empty public key.");

    std::cout << "[PASS] NIST P-256 key pair generated.\n"
              << "[INFO] Public key size : " << publicKey.size() << " bytes\n";
}

// -------------------------------------------------------------------------
// TEST 2 - Generate brainpoolP256r1 in another ECC slot
// -------------------------------------------------------------------------
void testGenerateBrainpool(DUOXTestContext &context)
{
    std::cout << "\n[TEST 2] Generate Brainpool P-256 R1 key pair...\n";

    const auto publicKey = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_BRAINPOOL,
        logicalaccess::DUOXManageKeyPairOption::GenerateKeyPair,
        logicalaccess::CurveID::BRAINPOOL_P256R1, DUOX_TEST_KEY_POLICY,
        DUOX_TEST_WRITE_ACCESS, DUOX_TEST_KUC_LIMIT);

    require(!publicKey.empty(), "DUOX Brainpool P-256 R1 generation returned an empty public key.");

    std::cout << "[PASS] Brainpool P-256 R1 key pair generated.\n"
              << "[INFO] Public key size : " << publicKey.size() << " bytes\n";
}

// -------------------------------------------------------------------------
// TEST 3 - Import a 32-byte P-256 private key
// -------------------------------------------------------------------------
void testImportP256(DUOXTestContext &context)
{
    std::cout << "\n[TEST 3] Import P-256 private key...\n";

    // P-256 private scalar d = 1, encoded as a 32-byte big-endian value
    const ByteVector privateKey{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

    require(privateKey.size() == 32, "Internal test error : P-256 private key must be exactly 32 bytes.");

    const auto result = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_IMPORT,
        logicalaccess::DUOXManageKeyPairOption::ImportPrivateKey,
        logicalaccess::CurveID::NIST_P256, DUOX_TEST_KEY_POLICY, DUOX_TEST_WRITE_ACCESS,
        DUOX_TEST_KUC_LIMIT, privateKey);

    // ImportPrivateKey is not expected to return a generated public key through this API
    require(result.empty(), "DUOX P-256 private key import returned unexpected response data.");

    std::cout << "[PASS] P-256 private key imported.\n";
}


// -------------------------------------------------------------------------
// TEST 4 - Update metadata of an existing ECC key
// -------------------------------------------------------------------------
void testUpdateMetadata(DUOXTestContext &context)
{
    std::cout << "\n[TEST 4] Update ECC key metadata...\n";

    /*
     * Test 4 is intentionally self-contained :
     * Ensure the dedicated metadata-test slot contains an ECC key before attempting to update its metadata
     * 
     * Do not rely on TEST 1/2 having been executed before this test !
     */

    std::cout << "[INFO] Preparing ECC key slot 0x" << std::hex
              << static_cast<unsigned int>(TEST_ECC_KEY_SLOT_METADATA) << std::dec
              << " for metadata update...\n";

    const auto publicKey = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_METADATA,
        logicalaccess::DUOXManageKeyPairOption::GenerateKeyPair,
        logicalaccess::CurveID::NIST_P256, DUOX_TEST_KEY_POLICY, DUOX_TEST_WRITE_ACCESS,
        DUOX_TEST_KUC_LIMIT);

    require(!publicKey.empty(), "DUOX metadata test setup failed : ECC key generation returned an empty public key.");

    std::cout << "[INFO] ECC key successfully created in dedicated metadata-test slot.\n";

    const auto result = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_METADATA,
        logicalaccess::DUOXManageKeyPairOption::UpdateMetadata,
        logicalaccess::CurveID::NIST_P256, DUOX_TEST_KEY_POLICY, DUOX_TEST_WRITE_ACCESS,
        DUOX_TEST_KUC_LIMIT);

    // UpdateMetadata is not expected to return any response data
    require(result.empty(), "ECC metadata update returned unexpected response data.");

    std::cout << "[PASS] ECC key metadata updated.\n";
}

// -------------------------------------------------------------------------
// TEST 5 - Create a CA Root Key
// -------------------------------------------------------------------------
void testCreateCARootKey(DUOXTestContext &context)
{
    // Exercise the interaction between ManageKeyPair and ManageCARootKey by generating a
    // public key and using it immediately as the CA Root Key input
    std::cout << "\n[TEST 5] Create CA Root Key...\n";

    /*
     * Generate a fresh P-256 key pair and use its public key as the input for ManageCARootKey
     *
     * This verifies that a public key returned by ManageKeyPair can be consumed directly by ManageCARootKey
     */
    const auto publicKey = context.duox()->manageKeyPair(
        TEST_ECC_KEY_SLOT_P256, logicalaccess::DUOXManageKeyPairOption::GenerateKeyPair,
        logicalaccess::CurveID::NIST_P256, DUOX_TEST_KEY_POLICY, DUOX_TEST_WRITE_ACCESS,
        DUOX_TEST_KUC_LIMIT);

    require(publicKey.size() == EXPECTED_P256_PUBLIC_KEY_SIZE,
            "DUOX CA Root Key test setup failed : generated P-256 public key must be exactly 65 bytes.");
    require(publicKey[0] == 0x04, "DUOX CA Root Key test setup failed : generated public "
                                  "key is not an uncompressed ECC public key.");

    context.duox()->manageCARootKey(
        TEST_CA_ROOT_KEY_SLOT, logicalaccess::CurveID::NIST_P256,
        DUOX_TEST_CA_ACCESS_RIGHTS, DUOX_TEST_CA_WRITE_ACCESS, DUOX_TEST_CA_READ_ACCESS,
        DUOX_TEST_CA_CRL_FILE, DUOX_TEST_CA_CRL_FILE_AID, publicKey, DUOX_TEST_CA_ISSUER);

    std::cout << "[PASS] CA Root Key created successfully.\n";
}

// -------------------------------------------------------------------------
// TEST 6 - Export ECC public key
// -------------------------------------------------------------------------
void testExportKey(DUOXTestContext &context)
{
    std::cout << "\n[TEST 6] Export ECC public key...\n";

    const auto key = context.duox()->exportKey(TEST_CA_ROOT_KEY_SLOT, DUOX_COMM_MODE);

    require(key.size() == EXPECTED_P256_PUBLIC_KEY_SIZE,
            "DUOX ExportKey returned an invalid P-256 public key size.");
    require(key[0] == 0x04,
        "DUOX ExportKey returned a P-256 public key which is not encoded as an uncompressed ECC point.");

    std::cout << "[PASS] DUOX P-256 public key exported successfully.\n"
              << "[INFO] Exported public key size : " << key.size() << " bytes\n";
}

// -------------------------------------------------------------------------
// TEST 7 - Get base key settings
// -------------------------------------------------------------------------
void testApplicationKeySettings(DUOXTestContext &context)
{
    std::cout << "\n[TEST 7] Get DUOX application key settings...\n";

    const auto settings = context.duox()->getKeySettings();

    // The test application was created with : createApplication(AID, KS_DEFAULT, 2, DF_KEY_AES)
    // Therefore it must contain exactly two AES keys
    requireAESKeySettings(settings, 2, false);
    // The application was created without an application key-set configuration
    // so the response should contain only the two base bytes

    require(settings.eccPrivateKeys.empty(),
            "DUOX base GetKeySettings unexpectedly returned ECC metadata.");
    require(settings.caRootKeys.empty(),
            "DUOX base GetKeySettings unexpectedly returned CA Root Key metadata.");

    std::cout << "[PASS] DUOX application key settings retrieved successfully.\n";
    std::cout << "[INFO] Number of keys : "
              << static_cast<unsigned int>(settings.numberOfKeys) << '\n';
    std::cout << "[INFO] Key type       : 0x" << std::hex
              << static_cast<unsigned int>(settings.keyType) << std::dec << '\n';
}

// -------------------------------------------------------------------------
// TEST 8 - Verify default GetKeySettings overload
// -------------------------------------------------------------------------
void testDefaultKeySettings(DUOXTestContext &context)
{
    std::cout << "\n[TEST 8] Verify default GetKeySettings overload...\n";

    const auto defaultSettings = context.duox()->getKeySettings();
    const auto explicitSettings = context.duox()->getKeySettings(GET_KEY_SETTINGS_DEFAULT);

    require(defaultSettings.responseType == explicitSettings.responseType,
            "DUOX GetKeySettings() and GetKeySettings(0x00) returned different response types.");
    require(defaultSettings.piccLevel == explicitSettings.piccLevel,
            "DUOX GetKeySettings() and GetKeySettings(0x00) returned different PICC-level state.");
    require(defaultSettings.keySettings == explicitSettings.keySettings,
            "DUOX GetKeySettings() and GetKeySettings(0x00) returned different KeySettings values.");
    require(defaultSettings.maxNoOfKeys == explicitSettings.maxNoOfKeys,
            "DUOX GetKeySettings() and GetKeySettings(0x00) returned different MaxNoOfKeys values.");
    require(defaultSettings.numberOfKeys == explicitSettings.numberOfKeys,
        "DUOX GetKeySettings() and GetKeySettings(0x00) returned different key counts.");
    require(defaultSettings.keyType == explicitSettings.keyType,
        "DUOX GetKeySettings() and GetKeySettings(0x00) returned different key types.");
    require(defaultSettings.hasApplicationKeySetSettings == explicitSettings.hasApplicationKeySetSettings,
            "DUOX GetKeySettings() and GetKeySettings(0x00) returned different application key-set state.");

    std::cout
        << "[PASS] Default GetKeySettings overload behaves identically to option 0x00.\n";
}

// -------------------------------------------------------------------------
// TEST 9 - Get ECC private key metadata
// -------------------------------------------------------------------------
void testECCMetadata(DUOXTestContext &context)
{
    // Note that the order of entries returned by GetKeySettings is not used as part of
    // the semantic test We don't explicitly identify each key by its KeyNo
    /*
     * The test application is expected to contain exactly four ECC private key metadata
     * entries : KeyNo 0 : generated NIST P-256 KeyNo 1 : generated Brainpool P-256 R1
     *   KeyNo 2 : imported NIST P-256
     *   KeyNo 3 : metadata update test key
     * Do not rely on the order in which the card returns these entries
     * The semantic identifier is entry.keyNo
     */
    std::cout << "\n[TEST 9] Get DUOX ECC private key metadata...\n";

    const auto settings = context.duox()->getKeySettings(GET_KEY_SETTINGS_ECC_PRIVATE_KEY);

    require(settings.responseType == logicalaccess::DUOXKeySettingsOption::ECCPrivateKeyMetadata,
            "DUOX ECC GetKeySettings returned an unexpected response type.");
    require(!settings.piccLevel,
            "DUOX ECC GetKeySettings incorrectly reports PICC level.");
    require(settings.eccPrivateKeys.size() == EXPECTED_ECC_METADATA_COUNT,
            "DUOX ECC metadata returned an unexpected number of entries : expected " +
                std::to_string(EXPECTED_ECC_METADATA_COUNT) + ", received " +
                std::to_string(settings.eccPrivateKeys.size()) + ".");

    const auto validate = [&](std::size_t index, std::uint8_t keyNo, logicalaccess::CurveID curve)
    {
        const auto &entry = settings.eccPrivateKeys[index];

        require(entry.keyNo == keyNo,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected key number.");
        require(entry.curveId == curve,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected CurveID.");
        require(entry.keyPolicy == DUOX_TEST_KEY_POLICY,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected KeyPolicy.");
        require(entry.writeAccess == DUOX_TEST_WRITE_ACCESS,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected WriteAccess.");
        require(entry.keyUsageCtrLimit == DUOX_TEST_KUC_LIMIT,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected KUC limit.");
        require(entry.keyUsageCtr == 0,
            "ECC metadata slot " + std::to_string(index) + " returned an unexpected KUC value.");
    };

    // Slot 0 : generated NIST P-256
    validate(0, TEST_ECC_KEY_SLOT_P256, logicalaccess::CurveID::NIST_P256);
    // Slot 1 : generated Brainpool P-256 R1
    validate(1, TEST_ECC_KEY_SLOT_BRAINPOOL, logicalaccess::CurveID::BRAINPOOL_P256R1);
    // Slot 2 : imported P-256
    validate(2, TEST_ECC_KEY_SLOT_IMPORT, logicalaccess::CurveID::NIST_P256);
    // Slot 3 : metadata update test key
    validate(3, TEST_ECC_KEY_SLOT_METADATA, logicalaccess::CurveID::NIST_P256);

    std::cout << "[PASS] DUOX ECC private key metadata retrieved and validated.\n";

    for (const auto &entry : settings.eccPrivateKeys)
    {
        std::cout << "[INFO] ECC slot " << static_cast<unsigned int>(entry.keyNo)
                  << " : CurveID=" << static_cast<unsigned int>(entry.curveId)
                  << ", KeyPolicy=0x" << std::hex << entry.keyPolicy << ", WriteAccess=0x"
                  << static_cast<unsigned int>(entry.writeAccess)
                  << ", KUC limit=" << std::dec << entry.keyUsageCtrLimit
                  << ", KUC=" << entry.keyUsageCtr << '\n';
    }
}

// -------------------------------------------------------------------------
// TEST 10 - Get CA Root Key metadata
// -------------------------------------------------------------------------
void testCARootMetadata(DUOXTestContext &context)
{
    std::cout << "\n[TEST 10] Get DUOX CA Root Key metadata...\n";

    const auto settings = context.duox()->getKeySettings(GET_KEY_SETTINGS_CA_ROOT_KEY);

    require(settings.responseType == logicalaccess::DUOXKeySettingsOption::CARootKeyMetadata,
            "DUOX CA Root Key GetKeySettings returned an unexpected response type.");
    require(!settings.piccLevel,
            "CA Root Key GetKeySettings incorrectly reports PICC level.");
    require(settings.caRootKeys.size() == EXPECTED_CA_ROOT_METADATA_COUNT,
        "DUOX CA Root Key metadata returned an unexpected number of entries : expected " +
            std::to_string(EXPECTED_CA_ROOT_METADATA_COUNT) + ", received " +
            std::to_string(settings.caRootKeys.size()) + ".");

    const auto &entry = settings.caRootKeys.front();

    require(entry.keyNo == TEST_CA_ROOT_KEY_SLOT,
            "DUOX CA Root Key metadata returned an unexpected key number.");
    require(entry.curveId == logicalaccess::CurveID::NIST_P256,
            "DUOX CA Root Key metadata returned an unexpected CurveID.");
    require(entry.accessRights == DUOX_TEST_CA_ACCESS_RIGHTS,
            "DUOX CA Root Key metadata returned unexpected AccessRights.");
    require(entry.writeAccess == DUOX_TEST_CA_WRITE_ACCESS,
            "DUOX CA Root Key metadata returned unexpected WriteAccess.");
    require(entry.readAccess == DUOX_TEST_CA_READ_ACCESS,
            "DUOX CA Root Key metadata returned unexpected ReadAccess.");
    require(entry.crlFile == DUOX_TEST_CA_CRL_FILE,
            "DUOX CA Root Key metadata returned unexpected CRLFile.");
    require(entry.crlFileAid == DUOX_TEST_CA_CRL_FILE_AID,
            "DUOX CA Root Key metadata returned unexpected CRLFileAID.");

    // These two assertions specifically exercise the semantic rule implemented by
    // getKeySettings() : CRL disabled => CRLFile == 0 and CRLFileAID == 0
    require(entry.crlFile == 0x00,
            "DUOX CA Root Key metadata indicates an unexpected CRL file.");
    require(entry.crlFileAid == 0x000000,
            "DUOX CA Root Key metadata indicates an unexpected CRL file AID.");

    std::cout << "[PASS] DUOX CA Root Key metadata retrieved and validated.\n";
    std::cout << "[INFO] CA Root Key slot : " << static_cast<unsigned int>(entry.keyNo) << '\n';
    std::cout << "[INFO] CurveID          : " << static_cast<unsigned int>(entry.curveId) << '\n';
    std::cout << "[INFO] AccessRights     : 0x" << std::hex << entry.accessRights << '\n';
    std::cout << "[INFO] WriteAccess      : 0x" << static_cast<unsigned int>(entry.writeAccess) << '\n';
    std::cout << "[INFO] ReadAccess       : 0x" << static_cast<unsigned int>(entry.readAccess) << '\n';
    std::cout << "[INFO] CRLFile          : 0x" << static_cast<unsigned int>(entry.crlFile) << '\n';
    std::cout << "[INFO] CRLFileAID       : 0x" << std::setw(6) << std::setfill('0')
              << entry.crlFileAid << std::dec << '\n';
}

// -------------------------------------------------------------------------
// TEST 11 - Cross-check CA Root Key metadata against ExportKey
// -------------------------------------------------------------------------
void testCARootMetadataExportConsistency(DUOXTestContext &context)
{
    std::cout << "\n[TEST 11] Cross-check CA Root Key metadata and ExportKey...\n";

    const auto metadata = context.duox()->getKeySettings(GET_KEY_SETTINGS_CA_ROOT_KEY);
    const auto exported = context.duox()->exportKey(TEST_CA_ROOT_KEY_SLOT, DUOX_COMM_MODE);
    const auto it = std::find_if(metadata.caRootKeys.begin(), metadata.caRootKeys.end(),
                                 [](const auto &entry)
                                 { return entry.keyNo == TEST_CA_ROOT_KEY_SLOT; });

    require(it != metadata.caRootKeys.end(),
            "CA Root Key metadata does not contain the exported slot.");
    require(it->curveId == logicalaccess::CurveID::NIST_P256,
            "CA Root Key metadata and configuration disagree on CurveID.");
    require(exported.size() == EXPECTED_P256_PUBLIC_KEY_SIZE,
            "ExportKey returned an unexpected public key size.");
    require(exported[0] == 0x04,
            "ExportKey returned a non-uncompressed P-256 public key.");

    std::cout << "[PASS] CA Root Key metadata is consistent with ExportKey.\n";
}

// -------------------------------------------------------------------------
// TEST 12 - PICC-level GetKeySettings
// -------------------------------------------------------------------------
void testPiccKeySettings(DUOXTestContext &context)
{
    std::cout << "\n[TEST 12] Get PICC-level DUOX key settings...\n";

    // This is a read-only operation
    // Selecting PICC level does not modify the card
    context.selectPicc();

    const auto settings = context.duox()->getKeySettings();

    // DUOX PICC level must contain exactly one AES key
    requireAESKeySettings(settings, 1, true);

    std::cout << "[PASS] PICC-level GetKeySettings validated successfully.\n";

    // Return to the dedicated test application immediately
    context.restoreTestAuthentication();
    // Selecting an application does not restore the previous EV2 session.
    // Re-authenticate explicitly before continuing with authenticated tests

    std::cout << "[PASS] Dedicated DUOX application reselected and EV2 authentication restored.\n";
}

// -------------------------------------------------------------------------
// TEST 13 - ChangeKey : replace a non-authenticated application key
// -------------------------------------------------------------------------
void testChangeKey(DUOXTestContext &context)
{
    std::cout << "\n[TEST 13] ChangeKey : replace a non-authenticated application key...\n";

    constexpr std::uint8_t KEY_SET_NO = 0;
    constexpr std::uint8_t KEY_NO     = SAFE_CHANGE_KEY_NO;

    context.assertChangeKeyContext(KEY_NO);

    // ---------------------------------------------------------------------
    // New key for application key 1
    // ---------------------------------------------------------------------
    const auto key = makeTestAESKey(CHANGE_KEY_VALUE_A, 0x01);

    std::cout << "[INFO] Changing ONLY application key " << static_cast<unsigned int>(KEY_NO) << "...\n";

    // ---------------------------------------------------------------------
    // Change key 1 while authenticated with key 0
    // ---------------------------------------------------------------------
    context.duox()->changeKey(KEY_NO, key);

    std::cout << "[PASS] Non-authenticated application key changed successfully.\n";

    const auto crypto = context.crypto();

    // Authentication with key 0 must remain valid
    requireEV2TestAuthentication(context);

    std::cout << "[PASS] Existing EV2 authentication remains valid.\n";

    // ---------------------------------------------------------------------
    // Verify host-side key store
    // ---------------------------------------------------------------------
    const auto stored = crypto->getKey(KEY_SET_NO, KEY_NO);

    requireNonNull(stored, "ChangeKey test : host-side key store was not updated.");
    require(stored->getKeyType() == logicalaccess::DF_KEY_AES,
            "ChangeKey test : host-side key has wrong type.");
    require(stored->getKeyVersion() == 0x01,
            "ChangeKey test : host-side key has wrong version.");
    require(stored->getData() == key->getData(),
        "ChangeKey test : host-side key material does not match the replacement key.");

    std::cout << "[PASS] Host-side crypto key store updated correctly.\n";

    // ---------------------------------------------------------------------
    // Verify card-side key by authenticating with key 1
    //
    // This is important : previous checks only prove that LogicalAccess updated its local key store
    // ---------------------------------------------------------------------
    std::cout << "[INFO] Authenticating with the newly changed key...\n";

    authenticateWithKey(context, KEY_NO, key);

    std::cout << "[PASS] Changed application key successfully authenticated.\n";

    // ---------------------------------------------------------------------
    // Restore the normal test authentication context : key 0.
    // ---------------------------------------------------------------------

    context.restoreTestAuthentication();

    std::cout << "[PASS] Authentication restored to application key 0.\n";
}

// -------------------------------------------------------------------------
// TEST 14 - ChangeKeyEV2: change application key 1 in keyset 0
// -------------------------------------------------------------------------
void testChangeKeyEV2(DUOXTestContext &context)
{
    std::cout << "\n[TEST 14] ChangeKeyEV2 end-to-end test...\n";

    constexpr std::uint8_t KEY_NO     = SAFE_CHANGE_KEY_NO;
    constexpr std::uint8_t KEY_SET_NO = 0;

    context.assertChangeKeyContext(KEY_NO);

    const auto key = makeTestAESKey(CHANGE_KEY_VALUE_B, 0x02);

    std::cout << "[INFO] Changing ONLY application key "
              << static_cast<unsigned int>(KEY_NO) << " in test keyset "
              << static_cast<unsigned int>(KEY_SET_NO) << "...\n";

    context.duox()->changeKeyEV2(KEY_SET_NO, KEY_NO, key);

    std::cout << "[PASS] ChangeKeyEV2 command completed.\n";

    // -------------------------------------------------------------
    // Host-side verification
    // -------------------------------------------------------------

    const auto stored = context.crypto()->getKey(KEY_SET_NO, KEY_NO);

    requireNonNull(stored, "ChangeKeyEV2 test : host-side key store was not updated.");
    require(stored->getKeyType() == logicalaccess::DF_KEY_AES,
            "ChangeKeyEV2 test : stored key is not AES.");
    require(stored->getKeyVersion() == 0x02,
            "ChangeKeyEV2 test : stored key has unexpected version.");

    // -------------------------------------------------------------
    // CARD-SIDE verification
    // -------------------------------------------------------------

    std::cout << "[INFO] Authenticating with key 1 after ChangeKeyEV2...\n";

    authenticateWithKey(context, KEY_NO, key);

    std::cout << "[PASS] ChangeKeyEV2 changed key authenticates successfully.\n";

    // -------------------------------------------------------------
    // Restore key-0 authentication.
    // -------------------------------------------------------------

    context.restoreTestAuthentication();

    std::cout << "[PASS] ChangeKeyEV2 test authentication restored.\n";
}

// -------------------------------------------------------------------------
// TEST 15 - ChangeKey / ChangeKeyEV2 interoperability sequence
// -------------------------------------------------------------------------
void testChangeKeyInteroperability(DUOXTestContext &context)
{
    std::cout << "\n[TEST 15] ChangeKey <-> ChangeKeyEV2 interoperability...\n";

    constexpr std::uint8_t KEY_NO     = SAFE_CHANGE_KEY_NO;
    constexpr std::uint8_t KEY_SET_NO = 0;

    context.assertChangeKeyContext(KEY_NO);

    const auto keyA = makeTestAESKey(CHANGE_KEY_VALUE_A, 0x10);
    const auto keyB = makeTestAESKey(CHANGE_KEY_VALUE_B, 0x11);
    const auto keyC = makeTestAESKey(CHANGE_KEY_VALUE_C, 0x12);

    // -------------------------------------------------------------
    // Step 1: ChangeKey
    // -------------------------------------------------------------

    std::cout << "[STEP 1] ChangeKey -> key A...\n";

    context.duox()->changeKey(KEY_NO, keyA);
    authenticateWithKey(context, KEY_NO, keyA);

    std::cout << "[PASS] ChangeKey -> key A.\n";

    // -------------------------------------------------------------
    // Return to key 0
    // -------------------------------------------------------------

    context.restoreTestAuthentication();

    // -------------------------------------------------------------
    // Step 2: ChangeKeyEV2
    // -------------------------------------------------------------

    std::cout << "[STEP 2] ChangeKeyEV2 -> key B...\n";

    context.duox()->changeKeyEV2(KEY_SET_NO, KEY_NO, keyB);
    authenticateWithKey(context, KEY_NO, keyB);

    std::cout << "[PASS] ChangeKeyEV2 -> key B.\n";

    // -------------------------------------------------------------
    // Return to key 0
    // -------------------------------------------------------------

    context.restoreTestAuthentication();

    // -------------------------------------------------------------
    // Step 3: ChangeKey again
    // -------------------------------------------------------------

    std::cout << "[STEP 3] ChangeKey -> key C...\n";

    context.duox()->changeKey(KEY_NO, keyC);
    authenticateWithKey(context, KEY_NO, keyC);

    std::cout << "[PASS] ChangeKey -> key C.\n";

    // -------------------------------------------------------------
    // IMPORTANT: restore key-0 authentication before anything else.
    // -------------------------------------------------------------

    context.restoreTestAuthentication();

    std::cout << "[PASS] ChangeKey / ChangeKeyEV2 interoperability verified.\n";
}

// ==============================================
// Test runner
// ==============================================

void createTestApplication(DUOXTestContext &context)
{
    // PICC-level operation : required for CreateApplication/DeleteApplication
    // PICC authentication is required to create an application
    // Re-select and re-authenticate after erase before creating the test application
    context.authenticatePicc();

    std::cout << "\n[INFO] Creating dedicated DUOX test application " << formatAid(DUOX_TEST_APPLICATION_AID) << "...\n";

    context.duox()->createApplication(DUOX_TEST_APPLICATION_AID, logicalaccess::KS_DEFAULT, 2, logicalaccess::DF_KEY_AES);

    std::cout << "[PASS] Dedicated DUOX test application created.\n";
}

} // namespace

void runDUOXManageKeyPairTests(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    DUOXTestContext context(chip);

    std::cout << "\n========================================\n";
    std::cout << " DUOX ManageKeyPair tests\n";
    std::cout << "========================================\n";

    // Safety configuration (must exist for each set of test)
    printTestSafetyPolicy();

    // 1 - Prepare the card
    prepareDUOXTestCard(context);

    // -------------------------------------------------------------------------
    // TEST 1 - FreeMem immediately after card preparation
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 1] Query DUOX free NV memory before test application creation...\n";

    /*
     * FreeMem is a PICC-level command
     *
     * The card has just been prepared and no DUOX test application has been created yet
     * This gives us a useful baseline for the memory lifecycle tests
     */
    const auto freeMemory = context.duox()->freeMem();

    std::cout << "[PASS] FreeMem succeeded.\n"
              << "[INFO] Free NV memory before test application: " << freeMemory << " bytes\n";

    // 2 - Create only the dedicated test application
    createTestApplication(context);

    DUOXTestApplicationGuard cleanup(context);

    // 3 - Select ONLY the dedicated application
    std::cout << "[INFO] Selecting test application " << formatAid(DUOX_TEST_APPLICATION_AID) << "...\n";

    // 4 - Verify the actual LogicalAccess application context
    context.selectTestApplication();

    const auto desfireChip = std::dynamic_pointer_cast<logicalaccess::DESFireChip>(chip);

    if (!desfireChip)
        throw std::runtime_error("DUOX card is not accessible as DESFireChip.");

    const auto crypto = context.crypto();

    if (!crypto)
        throw std::runtime_error("DESFire crypto context is not initialized.");

    std::cout << "[INFO] Current AID after selection : "
              << formatAid(crypto->d_currentAid) << '\n';

    std::cout << "[PASS] Correct DUOX test application selected.\n";

    // 5 - Authenticate ONLY to the test application
    std::cout << "[INFO] Authenticating to the dedicated DUOX application...\n";

    context.authenticateTestApplication();

    std::cout << "[PASS] Application authentication succeeded.\n";

    // 6 - Verify AID again after authentication
    const auto cryptoAfterAuth = context.crypto();

    if (!cryptoAfterAuth)
        throw std::runtime_error("DESFire crypto context is not initialized after authentication.");

    if (cryptoAfterAuth->d_currentAid != DUOX_TEST_APPLICATION_AID)
        throw std::runtime_error("SAFETY FAILURE : current AID changed after authentication.");

    std::cout << "[PASS] Authenticated context is still the dedicated DUOX test application.\n";

    // 7 - Only now run ManageKeyPair tests
    std::cout << "\n[SAFETY] All preconditions satisfied.\n"
              << "[SAFETY] ManageKeyPair tests may now modify ECC keys ONLY inside the dedicated application.\n\n";

    /*
     * ManageKeyPair tests intentionally use Full communication mode
     * The test application is dedicated to these operations and its ECC write access is configured accordingly
     *
     * Tests covering Plain and MAC communication modes should be added separately
     * rather than mixing communication-mode coverage into these functional tests
     */
    // Authentication / ManageKeyPair
    testAuthenticateEV2NonFirst(context);
    testGenerateP256(context);
    testGenerateBrainpool(context);
    testImportP256(context);
    testUpdateMetadata(context);

    // CA Root Key
    testCreateCARootKey(context);
    testExportKey(context);

    // GetKeySettings
    testApplicationKeySettings(context);
    testDefaultKeySettings(context);
    testECCMetadata(context);
    testCARootMetadata(context);
    testCARootMetadataExportConsistency(context);
    testPiccKeySettings(context);

    // ChangeKey
    testChangeKey(context);
    testChangeKeyEV2(context);
    testChangeKeyInteroperability(context);
    
    std::cout << "\n========================================\n"
              << " DUOX ManageKeyPair tests completed\n"
              << "========================================\n";
}

// ==============================================
// Main
// ==============================================

int main()
{
    /*
     * SAFETY RULES
     *
     * 1 - PICC selection is allowed ONLY for authentication required to create/delete DUOX_TEST_APPLICATION_AID
     * 2 - PICC Master Key is never modified.
     *     PICC Master Key authentication is allowed only for creation/deletion of DUOX_TEST_APPLICATION_AID
     * 3 - ChangeKey may modify only application key 1
     * 4 - Never call SetConfiguration()
     * 5 - Never call InitializeKeySet()
     * 6 - Never call RollKeySet()
     * 7 - ONLY create/delete DUOX_TEST_APPLICATION_AID
     * 8 - NEVER create or delete any other application
     * 9 - All ECC operations are restricted to DUOX_TEST_APPLICATION_AID
     * 10 - The dedicated test application is deleted after the tests
     */

    // EraseCard provides a clean NV-memory baseline for the test suite
    // The DeleteTestApplication mode is intended for cases where preserving unrelated card content is required
    try
    {
        if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
        {
            std::cout << "[WARNING] CARD_PREPARATION_MODE = EraseCard\n"
                      << "[WARNING] ALL applications and files will be deleted.\n";
        }
        else
        {
            std::cout << "[INFO] CARD_PREPARATION_MODE = DeleteTestApplication\n"
                      << "[INFO] Only the dedicated test application may be deleted.\n";
        }

        const auto readerConfiguration = createReaderConfiguration();
        const auto chip = waitForCard(readerConfiguration);
        const auto reader = readerConfiguration->getReaderUnit();

        requireNonNull(reader, "Reader disappeared after card connection.");

        std::cout << "[INFO] Card inserted on reader \"" << reader->getConnectedName() << "\"." << '\n';
        const auto uid = reader->getNumber(chip);

        std::cout << "[INFO] Card type : " << chip->getCardType() << '\n'
                  << "[INFO] Card UID  : " << logicalaccess::BufferHelper::getHex(uid) << '\n';

        if (!isDUOXCard(chip))
        {
            std::cout << "[INFO] Card is not a DUOX card. Nothing will be modified.\n";
            reader->disconnect();
            return EXIT_SUCCESS;
        }

        std::cout << "[INFO] Card is a DUOX card.\n";

        runDUOXManageKeyPairTests(chip);

        reader->disconnect();
        std::cout << "[INFO] Tests completed successfully." << std::endl;
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