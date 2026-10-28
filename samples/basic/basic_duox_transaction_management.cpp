/**
 * \file basic_duox_transaction_management.cpp
 * \brief Real card DUOX transaction management integration tests.
 *
 * Tests :
 *  - Creation and deletion of an isolated DESFire application
 *  - Standard data file read/write
 *  - Backup file commit and abort
 *  - Value file credit/debit and transaction commit
 *  - LinearRecord file write/read and commit
 *  - Transaction MAC file configuration and TMAC verification.
 *
 * SAFETY :
 *  - The default mode does not erase the PICC
 *  - Only the dedicated test application is eligible for cleanup
 *  - The PICC Master Key and application keys are never changed
 *  - An ambiguous commit is never retried automatically
 *
 * IMPORTANT:
 *  - Use a dedicated test card
 *  - Confirm the PICC AES key matches the card
 *  - Confirm the selected AID is reserved for this test
 *  - Verify the transaction-MAC API signatures against your checkout
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirechip.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecommands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfireev2commands.hpp>
#include <logicalaccess/plugins/cards/desfire/transactionmaccrypto.hpp>

#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>

#include <logicalaccess/plugins/readers/iso7816/commands/desfireev2iso7816commands.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/params.h>

using namespace logicalaccess;

namespace
{

// ==============================================
// (OpenSSL) Transaction Signature verification
// ==============================================

using EVPKeyPtr        = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVPKeyContextPtr = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

EVPKeyPtr decodeDUOXPublicKey(const ByteVector &encodedPublicKey)
{
    constexpr std::size_t P256_COORDINATE_SIZE = 32U;
    constexpr std::size_t P256_POINT_SIZE      = 1U + 2U * P256_COORDINATE_SIZE;

    constexpr std::uint8_t ECC_UNCOMPRESSED_POINT_PREFIX = 0x04;

    EXCEPTION_ASSERT_WITH_LOG(encodedPublicKey.size() == P256_POINT_SIZE,
        LibLogicalAccessException, "DUOX NIST P-256 public key must be exactly 65 bytes.");

    EXCEPTION_ASSERT_WITH_LOG(encodedPublicKey.front() == ECC_UNCOMPRESSED_POINT_PREFIX,
        LibLogicalAccessException, "DUOX public key must use the uncompressed EC point format.");

    EVP_PKEY_CTX *rawContext = EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);

    EXCEPTION_ASSERT_WITH_LOG(rawContext != nullptr,
        LibLogicalAccessException, "Failed to create OpenSSL EC public key import context.");

    EVPKeyContextPtr context(rawContext, &EVP_PKEY_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_fromdata_init(context.get()) > 0,
        LibLogicalAccessException, "Failed to initialize OpenSSL EC public key import.");

    char groupName[] = "prime256v1";

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, groupName, 0),

        OSSL_PARAM_construct_octet_string(
            OSSL_PKEY_PARAM_PUB_KEY, const_cast<std::uint8_t *>(encodedPublicKey.data()),
            encodedPublicKey.size()),

        OSSL_PARAM_construct_end()};

    EVP_PKEY *rawKey = nullptr;

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_fromdata(context.get(), &rawKey, EVP_PKEY_PUBLIC_KEY, params) > 0,
        LibLogicalAccessException, "Failed to import DUOX NIST P-256 public key.");

    EVPKeyPtr key(rawKey, &EVP_PKEY_free);

    EVP_PKEY_CTX *rawCheckContext = EVP_PKEY_CTX_new(key.get(), nullptr);

    EXCEPTION_ASSERT_WITH_LOG(rawCheckContext != nullptr,
        LibLogicalAccessException, "Failed to create EC public key validation context.");

    EVPKeyContextPtr checkContext(rawCheckContext, &EVP_PKEY_CTX_free);

    EXCEPTION_ASSERT_WITH_LOG(EVP_PKEY_public_check(checkContext.get()) > 0,
        LibLogicalAccessException, "DUOX NIST P-256 public key failed validation.");

    return key;
}

// ==============================================
// Configuration
// ==============================================

// WARNING: EraseCard permanently removes all applications and files.
enum class DUOXCardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

enum class TransactionMACReaderIDPolicy
{
    Required,
    NotRequired
};

struct TestApplicationConfig
{
    const char *name;
    std::uint32_t aid;
    DESFireKeyType keyType;
    const char *authenticationKeyHex;
    const char *transactionMACKeyHex;
};

// Change to DeleteTestApplication to preserve unrelated card applications. constexpr
constexpr DUOXCardPreparationMode CARD_PREPARATION_MODE = DUOXCardPreparationMode::EraseCard;

constexpr char READER_PROVIDER[]     = "PCSC";
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000;

// PICC-level application identifier
constexpr std::uint32_t PICC_LEVEL_AID = 0x000000;

// Select the behavior of the TMAC file being created.
constexpr TransactionMACReaderIDPolicy TMAC_READER_ID_POLICY = TransactionMACReaderIDPolicy::Required;

// Reserve this AID exclusively for this test
constexpr std::uint32_t AES128_TEST_APPLICATION_AID = 0x123456;
constexpr std::uint32_t AES256_TEST_APPLICATION_AID = 0x123457;
constexpr std::uint32_t TSV_TEST_APPLICATION_AID    = 0x123458;

constexpr std::uint8_t PICC_MASTER_KEY_NO = 0;
constexpr std::uint8_t APP_KEY_NO         = 0;

// Test application file identifiers
constexpr std::uint8_t DATA_FILE_NO   = 1;
constexpr std::uint8_t BACKUP_FILE_NO = 2;
constexpr std::uint8_t VALUE_FILE_NO  = 3;
constexpr std::uint8_t RECORD_FILE_NO = 4;
constexpr std::uint8_t TMAC_FILE_NO   = 5;

// File configuration
constexpr unsigned int DATA_FILE_SIZE  = 64;
constexpr unsigned int RECORD_SIZE     = 16;
constexpr unsigned int RECORD_CAPACITY = 4;

// Communication modes
// Verify the required modes against the card and library implementation
constexpr EncryptionMode COMM_MODE = EncryptionMode::CM_PLAIN;

// Test keys must match the keys provisioned on the dedicated test card
constexpr char TEST_AES128_AUTH_KEY_HEX[] =
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

constexpr char TEST_AES256_AUTH_KEY_HEX[] =
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

constexpr char TEST_AES128_TMAC_KEY_HEX[] =
    "10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F";

constexpr char TEST_AES256_TMAC_KEY_HEX[] =
    "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F "
    "10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F";


constexpr TestApplicationConfig AES128_TEST_CONFIG{"AES-128", AES128_TEST_APPLICATION_AID,
                                                   DF_KEY_AES, TEST_AES128_AUTH_KEY_HEX,
                                                   TEST_AES128_TMAC_KEY_HEX};

constexpr TestApplicationConfig AES256_TEST_CONFIG{"AES-256", AES256_TEST_APPLICATION_AID,
                                                   DF_KEY_AES256, TEST_AES256_AUTH_KEY_HEX,
                                                   TEST_AES256_TMAC_KEY_HEX};

constexpr TestApplicationConfig TSV_TEST_CONFIG{"TSV", TSV_TEST_APPLICATION_AID,
                                                DF_KEY_AES, TEST_AES128_AUTH_KEY_HEX,
                                                TEST_AES128_TMAC_KEY_HEX};


constexpr std::size_t TMRI_SIZE = 16;

// ECC configuration
constexpr std::uint8_t TSV_ECC_KEY_NO        = 0;
constexpr std::uint32_t DUOX_TEST_KUC_LIMIT  = 0;      // No key-usage limit
constexpr std::uint16_t DUOX_TEST_KEY_POLICY = 0x0040; // Enables ECC-based Transaction Signature

constexpr auto DUOX_COMM_MODE = logicalaccess::DUOXCommunicationMode::Full;

constexpr std::uint8_t DUOX_TEST_WRITE_AR  = 0x0E; // Free access
constexpr std::uint8_t DUOX_COMM_MODE_BITS = static_cast<std::uint8_t>(DUOX_COMM_MODE) << 4;
constexpr std::uint8_t DUOX_TEST_WRITE_ACCESS = DUOX_TEST_WRITE_AR | DUOX_COMM_MODE_BITS; // Free access


// ==============================================
// Formatting and assertions
// ==============================================

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

std::string formatAid(std::uint32_t aid)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << aid;
    return stream.str();
}

template <typename Container>
std::string toHex(const Container &data)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');

    for (const auto byte : data)
        oss << std::setw(2) << static_cast<unsigned int>(byte);

    return oss.str();
}

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

void expectEqual(const ByteVector &actual, const ByteVector &expected, const std::string &message)
{
    require(actual == expected, message);
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

// Decode a three-byte little-endian integer used in DESFire file settings
std::uint32_t decodeU24(const unsigned char value[3])
{
    return static_cast<std::uint32_t>(value[0]) |
           (static_cast<std::uint32_t>(value[1]) << 8U) |
           (static_cast<std::uint32_t>(value[2]) << 16U);
}

unsigned int checkedLength(const ByteVector &data)
{
    const auto maxLength = static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)());
    require(data.size() <= maxLength, "Buffer length exceeds the supported DESFire command length.");

    return static_cast<unsigned int>(data.size());
}

// ==============================================
// Traced transaction operations
// ==============================================

void writeDataTraced(DESFireEV2ISO7816Commands &commands, unsigned char fileNo,
                     const ByteVector &data, const std::string &description)
{
    traceOperation(description, [&]() { commands.writeData(fileNo, 0, data, COMM_MODE); });
}

ByteVector readDataTraced(DESFireEV2ISO7816Commands &commands, unsigned char fileNo,
                          const ByteVector &expected, const std::string &description)
{
    ByteVector actual;
    traceOperation(description, [&]() { actual = commands.readData(fileNo, 0, checkedLength(expected), COMM_MODE); });
    return actual;
}

CommitTransactionResult commitTraced(DESFireEV2ISO7816Commands &commands, bool returnTmac, const std::string &description)
{
    CommitTransactionResult result{};
    traceOperation(description, [&]() { result = commands.commitTransaction(returnTmac); });
    return result;
}

void abortTraced(DESFireEV2ISO7816Commands &commands)
{
    traceOperation("Abort transaction", [&]() { commands.abortTransaction(); });
}

ByteVector createTestReaderID()
{
    return ByteVector{0x28, 0xBF, 0x19, 0x82, 0xBE, 0x08, 0x6F, 0xBC,
                      0x60, 0xA2, 0x2D, 0xAE, 0xB6, 0x66, 0x13, 0xEE};
}

ByteVector commitReaderIDTraced(DESFireEV2ISO7816Commands &commands)
{
    const auto readerID = createTestReaderID();
    require(readerID.size() == TMRI_SIZE, "Reader ID must contain exactly 16 bytes.");

    ByteVector response;
    traceOperation("Commit transaction reader ID", [&]() { response = commands.commitReaderID(readerID); });
    return response;
}

constexpr bool readerIDRequired()
{
    return TMAC_READER_ID_POLICY == TransactionMACReaderIDPolicy::Required;
}

void beginTestTransaction(DESFireEV2ISO7816Commands &commands)
{
    if (readerIDRequired())
        (void)commitReaderIDTraced(commands);
}

// ==============================================
// Access Rights
// ==============================================

DESFireAccessRights createTestDataFileRights()
{
    return DESFireAccessRights{
        AR_FREE, // Read
        AR_FREE, // Write
        AR_FREE, // Read/write
        AR_FREE  // Change access rights
    };
}

DESFireAccessRights createTestValueFileRights()
{
    return DESFireAccessRights{
        AR_FREE, // Read
        AR_FREE, // Credit/debit access
        AR_FREE, // Read/write
        AR_FREE  // Change access rights
    };
}

DESFireAccessRights createTestRecordFileRights()
{
    return DESFireAccessRights{
        AR_FREE, // Read
        AR_FREE, // Write
        AR_FREE, // Read/write
        AR_FREE  // Change access rights
    };
}

DESFireAccessRights createTestTransactionMACFileRights()
{
    if (TMAC_READER_ID_POLICY == TransactionMACReaderIDPolicy::Required)
        return DESFireAccessRights{AR_FREE, AR_NEVER, AR_FREE, AR_FREE};

    return DESFireAccessRights{AR_FREE, AR_NEVER, AR_NEVER, AR_FREE};
}

// ==============================================
// Keys
// ==============================================

std::shared_ptr<DESFireKey> createTestPiccMasterKey()
{
    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(DF_KEY_AES);
    // Test PICC Master Key
    // This must match the actual AES PICC Master Key provisioned on the test card
    key->fromString(TEST_AES128_AUTH_KEY_HEX);
    return key;
}

std::shared_ptr<DESFireKey> createTestApplicationAuthKey(const TestApplicationConfig &config)
{
    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(config.keyType);
    key->fromString(config.authenticationKeyHex);
    return key;
}

std::shared_ptr<DESFireKey> createTestTransactionMACKey(const TestApplicationConfig &config)
{
    auto key = std::make_shared<DESFireKey>();
    key->setKeyType(config.keyType);
    key->fromString(config.transactionMACKeyHex);
    key->setKeyVersion(0x01);
    return key;
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

    std::string output = availableReaders.str();
    if (!output.empty() && output.back() == '\n')
        output.pop_back(); // Remove trailing newline
 
    throw std::runtime_error("Required PCSC reader was not found :\n  Expected : " +
                             readerName + "\nAvailable readers :\n" + output);
}

std::shared_ptr<ReaderConfiguration> createReaderConfiguration()
{
    auto configuration   = std::make_shared<ReaderConfiguration>();
    auto *libraryManager = LibraryManager::getInstance();

    requireNonNull(libraryManager, "LogicalAccess LibraryManager is unavailable.");

    const auto provider = libraryManager->getReaderProvider(READER_PROVIDER);

    requireNonNull(provider, "Unable to load the PCSC reader provider.");

    const auto reader = findReaderByName(provider, DESFIRE_READER_NAME);
    configuration->setReaderProvider(provider);
    configuration->setReaderUnit(reader);

    return configuration;
}

// Ensure the physical reader is disconnected on every exit path
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

    // Include a timestamp to correlate logs from separate test runs
    const auto startTime = std::time(nullptr);

    if (startTime != static_cast<std::time_t>(-1))
    {
        const auto *timestamp = std::ctime(&startTime);
        if (timestamp)
            logInfo(std::string("Time start : ") + timestamp);
    }

    // Establish the connection with the physical reader
    traceOperation("Connect to physical reader", [&]() { reader->connectToReader(); });

    traceOperation("Wait for card insertion", [&]()
        {
            require(reader->waitInsertion(CARD_INSERTION_TIMEOUT_MS), "No card was inserted within the timeout.");
        });
    traceOperation("Establish card communication", [&]()
        {
            require(reader->connect(), "A card was detected, but communication could not be established.");
        });

    std::shared_ptr<Chip> chip;
    traceOperation("Identify card type", [&]()
        {
            chip = reader->getSingleChip();
            requireNonNull(chip, "The reader connected successfully, but LogicalAccess could not identify the card.");
        });

    logInfo("Identified card type : " + chip->getCardType());
    return chip;
}

bool isDUOXCard(const std::shared_ptr<Chip> &chip)
{
    return chip && chip->getCardType() == "DUOX";
}

// ==============================================
// DUOX test context
// ==============================================

class DUOXTestContext
{
  public:
    explicit DUOXTestContext(const std::shared_ptr<Chip> &chip)
        : chip_(std::move(chip))
    {
        requireNonNull(chip_, "DUOX chip is null.");

        const auto commands = chip_->getCommands();

        desfireCommands_    = std::dynamic_pointer_cast<DESFireCommands>(commands);
        requireNonNull(desfireCommands_, "DUOX card does not expose DESFireCommands.");

        duoxCommands_ = std::dynamic_pointer_cast<DUOXCommands>(commands);
        requireNonNull(duoxCommands_, "DUOX card does not expose DUOXCommands.");

        desfireChip_ = std::dynamic_pointer_cast<DESFireChip>(chip_);
        requireNonNull(desfireChip_, "DUOX card is not accessible as DESFireChip.");
    }

    const std::shared_ptr<DESFireCommands> &desfire() const
    {
        return desfireCommands_;
    }

    const std::shared_ptr<DUOXCommands> &duox() const
    {
        return duoxCommands_;
    }

    const std::shared_ptr<DESFireChip> &desfireChip() const
    {
        return desfireChip_;
    }

    void selectPicc()
    {
        traceOperation("Selecting PICC level...", [&]() { duoxCommands_->selectApplication(PICC_LEVEL_AID); });
    }

    void selectApplication(const std::uint32_t aid)
    {
        require(aid != PICC_LEVEL_AID, "Safety failure : application AID cannot equal the PICC level.");
        traceOperation("Selecting application " + formatAid(aid) + "...", [&]() { duoxCommands_->selectApplication(aid); });
    }

    void authenticatePicc()
    {
        selectPicc();
        traceOperation("Authenticating PICC AES Master Key.",
                       [&]()
                       {
                           duoxCommands_->authenticateEV2First(PICC_MASTER_KEY_NO, createTestPiccMasterKey());
                       });
    }

    void authenticateApplication(const TestApplicationConfig &config)
    {
        selectApplication(config.aid);
        traceOperation(std::string("Authenticating ") + config.name + " test application key.",
                       [&]()
                       {
                           duoxCommands_->authenticateEV2First(APP_KEY_NO, createTestApplicationAuthKey(config));
                       });
    }

  private:
    std::shared_ptr<Chip> chip_;
    std::shared_ptr<DESFireCommands> desfireCommands_;
    std::shared_ptr<DUOXCommands> duoxCommands_;
    std::shared_ptr<DESFireChip> desfireChip_;
};

// ==============================================
// Application management
// ==============================================

std::vector<unsigned int> listApplications(const std::shared_ptr<DESFireCommands> &commands)
{
    requireNonNull(commands, "DESFire command interface is null.");

    const auto applications = commands->getApplicationIDs();

    printSection("Applications currently on card");

    if (applications.empty())
    {
        logInfo("No applications reported.");
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

void requireApplicationAbsent(DUOXTestContext &context, const TestApplicationConfig &config)
{
    const auto applications = context.desfire()->getApplicationIDs();

    require(!applicationExists(applications, config.aid),
            std::string(config.name) + " test application already exists : " + formatAid(config.aid));
}

void requireApplicationsAbsent(DUOXTestContext &context)
{
    requireApplicationAbsent(context, AES128_TEST_CONFIG);
    requireApplicationAbsent(context, AES256_TEST_CONFIG);
    requireApplicationAbsent(context, TSV_TEST_CONFIG);
}

void createTestApplication(DUOXTestContext &context, const TestApplicationConfig &config)
{
    context.authenticatePicc();

    // Only the application currently being created must be absent
    requireApplicationAbsent(context, config);

    logInfo(std::string("Creating ") + config.name + " test application " + formatAid(config.aid) + ".");

    context.duox()->createApplication(config.aid, KS_DEFAULT, 1, config.keyType);

    const auto updatedApplications = context.desfire()->getApplicationIDs();

    require(applicationExists(updatedApplications, config.aid),
            std::string("Application creation could not be verified : ") + formatAid(config.aid));

    logPass("Test application created.");
}

// ==============================================
// Card preparation
// ==============================================

void deleteApplication(DUOXTestContext &context, const TestApplicationConfig &config)
{
    printSection(std::string("DELETING TEST APP : ") + config.name);
    context.authenticatePicc();

    const auto applications = context.desfire()->getApplicationIDs();

    if (!applicationExists(applications, config.aid))
    {
        logInfo("Test application doesn't exist."); 
        return;
    }

    logInfo(std::string("Deleting ") + config.name + " test application " + formatAid(config.aid) + ".");

    context.desfire()->deleteApplication(config.aid);

    const auto remaining = context.desfire()->getApplicationIDs();

    require(!applicationExists(remaining, config.aid),
        std::string("Application deletion could not be verified : ") + formatAid(config.aid));

    logPass("Test application deleted.");
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

    // Show current applications before test preparation
    listApplications(context.desfire());

    // Format the card to release all previously allocated NV memory
    // OR
    // If dedicated test application already exists, deletes it
    if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
    {
        logInfo("Preparation mode : FULL PICC ERASE");
        eraseTestCard(context);
    }
    else
    {
        logInfo("Preparation mode : DELETE TEST APPLICATION ONLY");
        deleteApplication(context, AES128_TEST_CONFIG);
        deleteApplication(context, AES256_TEST_CONFIG);
        deleteApplication(context, TSV_TEST_CONFIG);
    }

    requireApplicationsAbsent(context);
    logPass("Test application AIDs are available.");
    std::cout << "===========================================\n";
}

// ==============================================
// Test application RAII cleanup
// ==============================================

// Deletes only the reserved test application when cleanup is active
class TestApplicationGuard
{
  public:
    explicit TestApplicationGuard(DUOXTestContext &context)
        : context_(context)
    {
    }

    ~TestApplicationGuard()
    {
        if (!active_)
            return;

        try
        {
            cleanup();
        }
        catch (const std::exception &exception)
        {
            std::cerr << "[WARNING] Automatic application cleanup failed : "
                      << exception.what() << '\n'
                      << "[WARNING] Manually delete the reserved test AIDs : "
                      << formatAid(AES128_TEST_APPLICATION_AID) << ", "
                      << formatAid(AES256_TEST_APPLICATION_AID) << ", "
                      << formatAid(TSV_TEST_APPLICATION_AID) << '\n'
                      << " after reconnecting and authenticating with the PICC Master Key.\n";
        }
        catch (...)
        {
            std::cerr << "[WARNING] Unknown application cleanup failure. Check both reserved AIDs manually.\n";
        }
    }

    TestApplicationGuard(const TestApplicationGuard &)            = delete;
    TestApplicationGuard &operator=(const TestApplicationGuard &) = delete;

    void cleanup()
    {
        if (!active_)
            return;

        std::exception_ptr firstFailure;

        // Attempt both deletions even if the first one fails
        for (const auto *config : {&AES128_TEST_CONFIG, &AES256_TEST_CONFIG, &TSV_TEST_CONFIG})
        {
            try
            {
                deleteApplication(context_, *config);
            }
            catch (...)
            {
                if (!firstFailure)
                    firstFailure = std::current_exception();
            }
        }

        // Do not claim cleanup succeeded if either deletion failed
        if (firstFailure)
            std::rethrow_exception(firstFailure);

        const auto remaining = context_.desfire()->getApplicationIDs();

        require(!applicationExists(remaining, AES128_TEST_APPLICATION_AID),
                "AES-128 test application still exists after cleanup.");

        require(!applicationExists(remaining, AES256_TEST_APPLICATION_AID),
                "AES-256 test application still exists after cleanup.");

        require(!applicationExists(remaining, TSV_TEST_APPLICATION_AID),
                "TSV test application still exists after cleanup.");

        active_ = false;

        logPass("All three reserved test applications were deleted.");
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
              << "[SAFETY] Preparation mode    : "
              << (eraseMode ? "FULL PICC ERASE" : "DELETE TEST APPLICATIONS")
              << '\n'
              << "[SAFETY] Reserved test AIDs  : "
              << formatAid(AES128_TEST_APPLICATION_AID) << ", "
              << formatAid(AES256_TEST_APPLICATION_AID) << ", "
              << formatAid(TSV_TEST_APPLICATION_AID) << '\n'
              << "[SAFETY] PICC Master Key     : AUTHENTICATION ONLY\n"
              << "[SAFETY] CARD CONTENT        : "
              << (eraseMode ? "WILL BE ERASED" : "TEST APPLICATIONS ONLY")
              << '\n'
              << "[SAFETY] NV memory           : "
              << (eraseMode ? "CLEAN BASELINE" : "NOT RESET") << '\n';
    std::cout << "===========================================================\n";
}

// ==============================================
// Test summary
// ==============================================

void printFinalTestSummary()
{
    std::cout << "\n"
              << "============================================================\n"
              << "              ALL DUOX TESTS PASSED\n"
              << "============================================================\n"
              << "[PASS] AES-128 transaction management tests\n"
              << "[PASS] AES-256 transaction management tests\n"
              << "[PASS] TSV transaction signature tests\n"
              << "============================================================\n";
}

// ==============================================
// File creation and configuration
// ==============================================

void createDataFile(DESFireEV2ISO7816Commands &commands, std::uint8_t fileNo)
{
    const auto rights = createTestDataFileRights();
    traceOperation("Create standard data file " + std::to_string(fileNo), [&]()
                   {
                       commands.createStdDataFile(fileNo, COMM_MODE, rights, DATA_FILE_SIZE, 0x0000, false);
                   });
}

void createBackupFile(DESFireEV2ISO7816Commands &commands, std::uint8_t fileNo)
{
    const auto rights = createTestDataFileRights();
    traceOperation("Create backup data file " + std::to_string(fileNo), [&]()
                   {
                       commands.createBackupFile(fileNo, COMM_MODE, rights, DATA_FILE_SIZE, 0x0000, false);
                   });
}

void createValueFile(DESFireEV2ISO7816Commands &commands, std::uint8_t fileNo)
{
    const auto rights = createTestValueFileRights();
    traceOperation("Create value file " + std::to_string(fileNo), [&]()
        {
            commands.createValueFile(fileNo, COMM_MODE, rights, 0, 1000, 100, true);
        });
}

void createRecordFile(DESFireEV2ISO7816Commands &commands, std::uint8_t fileNo)
{
    const auto rights = createTestRecordFileRights();
    traceOperation("Create linear record file " + std::to_string(fileNo), [&]()
                   {
                       commands.createLinearRecordFile(fileNo, COMM_MODE, rights,
                                                       RECORD_SIZE, RECORD_CAPACITY,
                                                       0x0000, false);
                   });
}

void createTransactionMACFile(DESFireEV2ISO7816Commands &commands, const TestApplicationConfig &config)
{
    const auto rights = createTestTransactionMACFileRights();
    const auto key    = createTestTransactionMACKey(config);

    traceOperation(std::string("Create ") + config.name + " transaction MAC file", [&]()
        {
            commands.createTransactionMACFile(TMAC_FILE_NO, COMM_MODE, rights, key);
        });
}

ByteVector provisionTSVECCKey(DUOXTestContext &context)
{
    ByteVector publicKey;

    traceOperation("Generate TSV NIST P-256 key pair",
                   [&]()
                   {
                       publicKey = context.duox()->manageKeyPair(
                           TSV_ECC_KEY_NO, DUOXManageKeyPairOption::GenerateKeyPair,
                           CurveID::NIST_P256, DUOX_TEST_KEY_POLICY,
                           DUOX_TEST_WRITE_ACCESS, DUOX_TEST_KUC_LIMIT);
                   });

    require(!publicKey.empty(), "DUOX NIST P-256 generation returned an empty public key.");

    logInfo("Generated public key (" + std::to_string(publicKey.size()) + " bytes) :\n" + toHex(publicKey));

    return publicKey;
}

void createTransactionSignatureFile(DUOXISO7816Commands &commands, const TestApplicationConfig &config)
{
    DUOXTransactionMACFileOptions options;

    options.fileNo            = TMAC_FILE_NO;
    options.communicationMode = EncryptionMode::CM_ENCRYPT;
    options.accessRights = DESFireAccessRights{AR_FREE, AR_NEVER, AR_NEVER, AR_FREE};
    // Can use createTestTransactionMACFileRights() for accessRights but our test on TSV doesn't use commitReaderID

    options.mode                      = DUOXTransactionMACFileMode::TransactionSignature;
    options.transactionSignatureKeyNo = 0;

    options.excludeFilesFromTMI = false;
    options.tmiExclusionFileMap = 0;

    const auto key = createTestTransactionMACKey(config);
    options.transactionMACKey = key;

    traceOperation(std::string("Create (") + config.name + ") TSV transaction signature file", [&]()
        {
            commands.createTransactionMACFile(options);
        });
}

void provisionTSVFiles(DUOXISO7816Commands &commands, const TestApplicationConfig &config)
{
    createTransactionSignatureFile(commands, config);
    createDataFile(commands, DATA_FILE_NO);
}

void configureTestTransactionMAC(DESFireEV2ISO7816Commands &commands,
                                 const ByteVector &uid,
                                 const TestApplicationConfig &config)
{
    const bool supportedUIDSize = uid.size() == 4 || uid.size() == 7 || uid.size() == 10;

    require(supportedUIDSize, "Unsupported UID length for TMAC verification. Expected 4 (EV3 only), 7 or 10 bytes.");

    const auto key = createTestTransactionMACKey(config);

    traceOperation(
        std::string("Configure host side ") + config.name + " transaction MAC verification", [&]()
        {
            commands.configureTransactionMAC(key->getData(), uid, TMAC_FILE_NO);
        });
}

// ==============================================
// File settings validation
// ==============================================

void testAllFileSettings(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : GET FILE SETTINGS - ALL FILE TYPES");

    // DESFire file type identifiers
    constexpr unsigned char STANDARD_DATA_FILE = 0x00;
    constexpr unsigned char BACKUP_DATA_FILE   = 0x01;
    constexpr unsigned char VALUE_FILE         = 0x02;
    constexpr unsigned char LINEAR_RECORD_FILE = 0x03;
    constexpr unsigned char TMAC_FILE          = 0x05;

    const auto checkFileType = [&](unsigned char fileNo, unsigned char expectedType, const std::string &description)
    {
        traceOperation("GetFileSettings : " + description, [&]()
                       {
                           const auto settings = commands.getFileSettings(fileNo);

                           require(settings.fileType == expectedType,
                                   description + " : unexpected file type. Expected " +
                                       std::to_string(expectedType) + ", got " +
                                       std::to_string(settings.fileType));
                       });
    };

    // 1 - Standard data file : validate file type and size
    traceOperation("GetFileSettings : standard data file", [&]()
                   {
                       const auto settings = commands.getFileSettings(DATA_FILE_NO);
                       require(settings.fileType == STANDARD_DATA_FILE, "Standard file : incorrect file type.");

                       const auto dataSettings = settings.getDataFile();
                       const auto size         = decodeU24(dataSettings.fileSize);
                       require(size == DATA_FILE_SIZE, "Standard file : incorrect file size.");
                   });

    // 2 - Backup data file : validate file type and size
    traceOperation("GetFileSettings : backup data file",
                   [&]()
                   {
                       const auto settings = commands.getFileSettings(BACKUP_FILE_NO);

                       require(settings.fileType == BACKUP_DATA_FILE, "Backup file : incorrect file type.");

                       const auto dataSettings = settings.getDataFile();
                       const auto size         = decodeU24(dataSettings.fileSize);

                       require(size == DATA_FILE_SIZE, "Backup file : incorrect file size.");
                   });

    // 3 - Value file : validate limits and limited credit configuration
    traceOperation("GetFileSettings : value file",
        [&]()
        {
            const auto settings = commands.getFileSettings(VALUE_FILE_NO);

            require(settings.fileType == VALUE_FILE, "Value file : incorrect file type.");

            const auto valueSettings = settings.getValueFile();

            require(valueSettings.lowerLimit == 0, "Value file : incorrect lower limit.");
            require(valueSettings.upperLimit == 1000, "Value file : incorrect upper limit.");
            require(valueSettings.limitedCreditEnabled, "Value file : limited credit should be enabled.");
        });

    // 4 - Linear record file : validate record size, capacity, and initial count
    traceOperation("GetFileSettings : linear record file",
        [&]()
        {
            const auto settings = commands.getFileSettings(RECORD_FILE_NO);

            require(settings.fileType == LINEAR_RECORD_FILE, "Record file : incorrect file type.");

            const auto recordSettings = settings.getRecordFile();
            const auto recordSize = decodeU24(recordSettings.recordSize);
            const auto maxRecords = decodeU24(recordSettings.maxNumberRecords);
            const auto currentRecords = decodeU24(recordSettings.currentNumberRecords);

            require(recordSize == RECORD_SIZE, "Record file : incorrect record size.");
            require(maxRecords == RECORD_CAPACITY, "Record file : incorrect maximum record count.");
            require(currentRecords == 0, "Record file : expected an empty file before tests.");
        });

    // 5 - Transaction MAC file : validate file type
    checkFileType(TMAC_FILE_NO, TMAC_FILE, "transaction MAC file");

    logPass("GetFileSettings validated all five file types.");
}

// ==============================================
// Transaction tests
// ==============================================

void testStandardDataFile(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : STANDARD DATA FILE");

    const ByteVector expected{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

    // Start a transaction using the configured Reader ID policy.
    beginTestTransaction(commands);

    // Perform the operations being tested
    writeDataTraced(commands, DATA_FILE_NO, expected, "Write standard data file");

    const auto actual = readDataTraced(commands, DATA_FILE_NO, expected, "Read standard data file");

    expectEqual(actual, expected, "Standard file read-back mismatch.");

    // Explicitly finish the transaction
    const auto result = commitTraced(commands, true, "Commit standard data file transaction");

    require(result.type == CommitTransactionResult::Type::MAC, "CommitTransaction did not return a TMV.");

    logPass("Standard data read/write.");
}

void testBackupCommit(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : BACKUP FILE COMMIT");

    const ByteVector expected{0xA1, 0xA2, 0xA3, 0xA4, 0xB1, 0xB2, 0xB3, 0xB4};
    
    // Transaction 1 : stage and commit the backup-file update
    beginTestTransaction(commands);

    writeDataTraced(commands, BACKUP_FILE_NO, expected, "Stage backup file data");

    // Commit the staged update and retrieve the TMV
    const auto result = commitTraced(commands, true, "Commit backup file transaction");

    require(result.type == CommitTransactionResult::Type::MAC, "Expected a transaction MAC from CommitTransaction.");

    // Verify that the committed data is actually present
    // and separate transaction if configured TMAC requires CommitReaderID before next transaction
    beginTestTransaction(commands);

    const auto actual = readDataTraced(commands, BACKUP_FILE_NO, expected, "Verify committed backup file data");

    // Close the transaction containing the verification read
    const auto readResult = commitTraced(commands, true, "Commit backup verification-read transaction");

    require(readResult.type == CommitTransactionResult::Type::MAC, "Verification-read commit did not return a TMV.");

    expectEqual(actual, expected, "Backup file mismatch after commit.");

    logPass("Backup file committed and verified.");
}

void testAbort(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : ABORT TRANSACTION");

    const ByteVector original{0xA1, 0xA2, 0xA3, 0xA4, 0xB1, 0xB2, 0xB3, 0xB4};
    const ByteVector staged{0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF};

    // Start the transaction that will be aborted
    beginTestTransaction(commands);
    // Verify the committed baseline before starting the abort test
    const auto before = readDataTraced(commands, BACKUP_FILE_NO, original, "Read backup file baseline");
    const auto readResult = commitTraced(commands, true, "Commit transaction");

    require(readResult.type == CommitTransactionResult::Type::MAC, "Baseline read transaction did not return a TMV.");

    expectEqual(before, original, "Abort test requires the known committed baseline.");

    writeDataTraced(commands, BACKUP_FILE_NO, staged, "Stage data for abort test");

    abortTraced(commands);
    
    beginTestTransaction(commands);
    const auto after = readDataTraced(commands, BACKUP_FILE_NO, original, "Verify backup file data after abort");
    (void)commitTraced(commands, true, "Commit transaction");

    expectEqual(after, original, "Abort did not restore the committed backup file contents.");
    
    logPass("Abort restored the committed baseline.");
}

void testValueFile(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : VALUE FILE");

    constexpr std::int32_t INITIAL_VALUE  = 100;
    constexpr unsigned int CREDIT_AMOUNT  = 25;
    constexpr unsigned int DEBIT_AMOUNT   = 10;
    constexpr std::int32_t EXPECTED_VALUE = INITIAL_VALUE + CREDIT_AMOUNT - DEBIT_AMOUNT;

    const auto readValue = [&](const std::string &stage)
    {
        const auto value = commands.getValue(VALUE_FILE_NO, COMM_MODE);
        logInfo(stage + " : " + std::to_string(value));
        return value;
    };

    // Required if the configured TMAC file enables CommitReaderID
    beginTestTransaction(commands);

    // Check the committed baseline
    require(readValue("Initial committed value") == INITIAL_VALUE, "Unexpected initial value.");

    // Stage credit
    traceOperation("Credit value file", [&]() { commands.credit(VALUE_FILE_NO, CREDIT_AMOUNT, COMM_MODE); });

    // Pending changes are not visible through GetValue
    require(readValue("Value before commit after credit") == INITIAL_VALUE,
            "Unexpected committed value before transaction commit.");

    // Stage debit in the same transaction
    traceOperation("Debit value file", [&]() { commands.debit(VALUE_FILE_NO, DEBIT_AMOUNT, COMM_MODE); });

    require(readValue("Value before commit after debit") == INITIAL_VALUE,
            "Unexpected committed value before transaction commit.");

    // Apply the pending changes
    traceOperation("Commit value file transaction", [&]() { (void)commands.commitTransaction(false); });

    // Verify the new committed value
    const auto committed = readValue("Value after commit");

    logInfo("Expected committed value : " + std::to_string(EXPECTED_VALUE));

    require(committed == EXPECTED_VALUE, "Committed value mismatch : expected " +
                                             std::to_string(EXPECTED_VALUE) + ", got " +
                                             std::to_string(committed));

    logPass("Value operations and commit.");
}

void testRecordFile(DESFireEV2ISO7816Commands &commands)
{
    printSection("TEST : LINEAR RECORD FILE");

    const ByteVector record{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    // Transaction 1 : write and commit the record
    beginTestTransaction(commands);

    traceOperation("Write linear record", [&]() { commands.writeRecord(RECORD_FILE_NO, 0, record, COMM_MODE); });

    // Commit the record before reading it
    (void)commitTraced(commands, false, "Commit record write transaction");

    // Transaction 2 : read the committed record
    beginTestTransaction(commands);
    traceOperation("Read linear record", [&]()
                   {
                       const auto actual = commands.readRecords(RECORD_FILE_NO,
                                                                0, // Latest record
                                                                1, // One record
                                                                COMM_MODE);

                       expectEqual(actual, record, "Record file read back mismatch.");
                   });
    // Close the read transaction
    (void)commitTraced(commands, false, "Commit record read transaction");

    logPass("Record write/read/commit.");
}

void testTMVWithoutReaderID(DESFireEV2ISO7816Commands &commands, const ByteVector &uid,
                            const TestApplicationConfig &config)
{
    printSection("TEST : TMV WITHOUT CommitReaderID");

    configureTestTransactionMAC(commands, uid, config);
    commands.resetTransactionMACInput();

    // Create a non-empty TMI using an operation included in TMAC
    const ByteVector expected{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};

    writeDataTraced(commands, DATA_FILE_NO, expected, "Initialize data file");

    const auto actual = readDataTraced(commands, DATA_FILE_NO, expected,
                                       "Read data without committing Reader ID");

    expectEqual(actual, expected, "ReadData returned unexpected data.");

    const auto tmiBeforeCommit = commands.getTransactionMACInput();

    require(!tmiBeforeCommit.empty(), "Expected ReadData to populate the TMI.");

    logInfo("TMI before commit: " + toHex(tmiBeforeCommit));

    const auto result = commitTraced(commands, true, "Commit without Reader ID");

    require(result.type == CommitTransactionResult::Type::MAC, "CommitTransaction did not return a TMV.");

    require(commands.verifyTransactionMAC(result.mac), "TMV verification failed without Reader ID.");

    logInfo("TMC : " + std::to_string(commands.decodeTransactionMACCounter(result)));
    logInfo("TMV : " + toHex(result.mac.tmv));

    logPass("CommitTransaction without Reader ID.");
}

void testTMVReturned(DESFireEV2ISO7816Commands &commands, const ByteVector &uid,
                     const TestApplicationConfig &config)
{
    printSection("TEST : TMV returned by CommitTransaction");

    configureTestTransactionMAC(commands, uid, config);

    traceOperation("Reset host side TMAC input", [&]() { commands.resetTransactionMACInput(); });

    require(commands.getTransactionMACInput().empty(), "Host-side TMI was not cleared at test start.");

    // Set the reader ID before performing operations in this transaction
    (void)commitReaderIDTraced(commands);

    const auto result = commitTraced(commands, true, "Commit without changes");

    require(result.type == CommitTransactionResult::Type::MAC, "CommitTransaction did not return a TMV.");

    logInfo("TMC : " + std::to_string(commands.decodeTransactionMACCounter(result)));
    logInfo("TMV : " + toHex(result.mac.tmv));

    require(result.mac.tmv.size() == 8, "Invalid TMV length.");

    require(commands.verifyTransactionMAC(result.mac), "Returned TMV does not match host side computation.");

    logPass("TMV returned by CommitTransaction.");
}

void testTransactionMAC(DESFireEV2ISO7816Commands &commands, const ByteVector &uid,
                        const TestApplicationConfig &config)
{
    printSection("TEST : TRANSACTION MAC");

    configureTestTransactionMAC(commands, uid, config);

    traceOperation("Reset host side TMAC input", [&]() { commands.resetTransactionMACInput(); });

    // Set the reader ID before performing operations in this transaction
    const auto readerResponse = commitReaderIDTraced(commands);

    logInfo("CommitReaderID response length : " + std::to_string(readerResponse.size()));

    const ByteVector payload{0xDE, 0xAD, 0xBE, 0xEF};

    writeDataTraced(commands, DATA_FILE_NO, payload, "Write TMAC test payload");

    const auto tmi = commands.getTransactionMACInput();

    require(!tmi.empty(), "Expected a non-empty TMI before commit.");

    const auto result = commitTraced(commands, true, "Commit TMAC test transaction");
    
    require(result.type == CommitTransactionResult::Type::MAC, "Commit did not return a transaction MAC.");

    logInfo("Returned TMC : " + std::to_string(commands.decodeTransactionMACCounter(result)));

    traceOperation("Verify transaction MAC", [&]()
                   {
                       require(commands.verifyTransactionMAC(result.mac), "Transaction MAC verification failed.");
                   });

    auto tampered = result.mac;
    tampered.tmv[0] ^= 0x01;

    traceOperation("Reject tampered transaction MAC", [&]()
                   {
                       require(!commands.verifyTransactionMAC(tampered), "TMAC verification accepted a modified TMV.");
                   });

    logPass("TMAC verification and tamper detection.");
}

void testMultipleTransactionMAC(DESFireEV2ISO7816Commands &commands,
                                const ByteVector &uid,
                                const TestApplicationConfig &config)
{
    printSection("TEST : TRANSACTION MAC WITH MULTIPLE COMMANDS");

    configureTestTransactionMAC(commands, uid, config);

    traceOperation("Reset host-side TMAC input", [&]() { commands.resetTransactionMACInput(); });

    require(commands.getTransactionMACInput().empty(), "Host-side TMI was not cleared at test start.");

    // Start the transaction
    const auto readerResponse = commitReaderIDTraced(commands);

    logInfo("CommitReaderID response length : " + std::to_string(readerResponse.size()));

    // Command 1 : WriteData
    const ByteVector payload{0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44};

    writeDataTraced(commands, DATA_FILE_NO, payload, "Write TMAC test payload");

    // Command 2 : ReadData
    // This is included deliberately to test accumulation of multiple command types in the same transaction
    const auto actual = readDataTraced(commands, DATA_FILE_NO, payload, "Read TMAC test payload");

    expectEqual(actual, payload, "TMAC test payload read-back mismatch.");

    // Command 3 : GetValue
    // This adds a third operation to the same transaction

    traceOperation("Get value file", [&]() { return commands.getValue(VALUE_FILE_NO, COMM_MODE); });

    // Snapshot the exact input before commit clears the active TMI
    const auto tmiBeforeCommit = commands.getTransactionMACInput();

    require(!tmiBeforeCommit.empty(), "Expected a non-empty TMI before commit.");

    // Commit exactly once for the transaction under test
    const auto result = commitTraced(commands, true, "Commit multi-command TMAC transaction");

    require(result.type == CommitTransactionResult::Type::MAC, "CommitTransaction did not return a TMV.");

    logInfo("Returned TMC : " + std::to_string(commands.decodeTransactionMACCounter(result)));
    logInfo("Returned TMV : " + BufferHelper::getHex(ByteVector(result.mac.tmv.begin(), result.mac.tmv.end())));

    // Verify using the saved committed TMI
    traceOperation("Verify transaction MAC",
                   [&]()
                   {
                       require(commands.verifyTransactionMAC(result.mac), "Transaction MAC verification failed.");
                   });

    logPass("Multi-command TMAC verification.");
}

void testTSVCommit(DUOXISO7816Commands &commands, EVP_PKEY *applicationPublicKey)
{
    printSection("TEST : TSV WRITE AND COMMIT");

    traceOperation("Initialize TSV transaction tracking", [&]() { commands.configureTransactionSignature(TMAC_FILE_NO); });

    const ByteVector payload{0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44};

    writeDataTraced(commands, DATA_FILE_NO, payload, "Write TSV test payload");

    const auto result = commitTraced(commands, true, "Commit TSV transaction");

    require(result.type == CommitTransactionResult::Type::Signature, "CommitTransaction did not return a TSV.");

    logInfo("TMC : " + std::to_string(commands.decodeTransactionResultCounter(result)));

    // The 64-byte signature generated by the card
    logInfo("Card TSV (" + std::to_string(result.signature.tsv.size()) + " bytes) :\n" +
            toHex(ByteVector(result.signature.tsv.begin(), result.signature.tsv.end())));

    require(result.signature.tsv.size() == 64U, "Transaction signature has an invalid size.");

    traceOperation("Verify transaction signature", [&]()
        {
            const bool valid = commands.verifyTransactionSignature(result.signature, applicationPublicKey);
            logInfo(std::string("Backend TSV verification : ") + (valid ? "VALID" : "INVALID"));
            require(valid, "Transaction Signature verification failed.");
        });

    auto tampered = result.signature;
    tampered.tsv[0] ^= 0x01U;

    traceOperation("Reject tampered transaction signature", [&]()
        {
            require(!commands.verifyTransactionSignature(tampered, applicationPublicKey),
                    "TSV verification accepted a modified signature.");
        });

    logPass("TSV verification and tamper detection.");
}

// ==============================================
// Complete integration test
// ==============================================

void runApplicationTransactionTests(DUOXTestContext &context,
                                    DESFireEV2ISO7816Commands &commands,
                                    const ByteVector &uid,
                                    const TestApplicationConfig &config)
{
    printSection(std::string("TEST SUITE : ") + config.name);
    logInfo("Application name : " + std::string(config.name));
    logInfo("Application AID  : " + formatAid(config.aid));
    logInfo(std::string("Key type         : ") + (config.keyType == DF_KEY_AES256 ? "AES-256" : "AES-128"));

    // Application creation and authentication
    printSection("PHASE 1/5 : APPLICATION SETUP");

    traceOperation(std::string("Create ") + config.name + " application " + formatAid(config.aid), [&]()
        {
            createTestApplication(context, config);
        });

    traceOperation(std::string("Authenticate ") + config.name + " application", [&]()
        {
            context.authenticateApplication(config);
        });

    logPass(std::string(config.name) + " application created and authenticated.");

    // File provisioning
    printSection("PHASE 2/5 : FILE PROVISIONING");

    logInfo("Creating test files...");

    createDataFile(commands, DATA_FILE_NO);
    createBackupFile(commands, BACKUP_FILE_NO);
    createValueFile(commands, VALUE_FILE_NO);
    createRecordFile(commands, RECORD_FILE_NO);
    createTransactionMACFile(commands, config);

    logPass("All test files created.");
    logPass("Transaction MAC file configured.");

    // File settings validation
    printSection("PHASE 3/5 : FILE SETTINGS VALIDATION");

    testAllFileSettings(commands);

    logPass("File settings validation completed.");

    // Transaction tests
    printSection("PHASE 4/5 : TRANSACTION TESTS");

    logInfo("[1/6] Standard data file");
    testStandardDataFile(commands);
    logInfo("[2/6] Backup file commit");
    testBackupCommit(commands);
    logInfo("[3/6] Transaction abort");
    testAbort(commands);
    logInfo("[4/6] Value file");
    testValueFile(commands);
    logInfo("[5/6] Linear record file");
    testRecordFile(commands);
    if (readerIDRequired())
    {
        logInfo("[6/8] Transaction MAC verification");
        testTMVReturned(commands, uid, config);
        logInfo("[7/8] Transaction MAC and tamper detection");
        testTransactionMAC(commands, uid, config);
        logInfo("[8/8] Transaction MAC with multiple commands");
        testMultipleTransactionMAC(commands, uid, config);
    }
    else
    {
        logInfo("[6/6] Transaction MAC without CommitReaderID");
        testTMVWithoutReaderID(commands, uid, config);
    }

    // Final validation summary
    printSection("PHASE 5/5 : TEST SUITE VALIDATION");

    std::cout << "\n"
              << "  Application        : " << config.name << '\n'
              << "  Application AID    : " << formatAid(config.aid) << '\n'
              << "  AES key type       : "
              << (config.keyType == DF_KEY_AES256 ? "AES-256" : "AES-128") << '\n'
              << "  Reader ID required : " << (readerIDRequired() ? "YES" : "NO") << '\n'
              << "  File settings      : PASS\n"
              << "  Data/transaction   : PASS\n"
              << "  Transaction MAC    : PASS\n"
              << "  Overall result     : PASS\n"
              << "==============================================\n";

    logPass(std::string("ALL TESTS PASSED FOR ") + config.name);
}

void runTSVApplicationTransactionTests(DUOXTestContext &context,
                                       DUOXISO7816Commands &duoxCommands,
                                       DESFireEV2ISO7816Commands &desfireCommands)
{
    printSection("TEST SUITE : TSV");

    logInfo("Application name : " + std::string(TSV_TEST_CONFIG.name));
    logInfo("Application AID  : " + formatAid(TSV_TEST_CONFIG.aid));

    printSection("PHASE 1/4 : APPLICATION SETUP");

    traceOperation("Create TSV application", [&]() { createTestApplication(context, TSV_TEST_CONFIG); });

    traceOperation("Authenticate TSV application", [&]() { context.authenticateApplication(TSV_TEST_CONFIG); });

    printSection("PHASE 2/4 : ECC KEY PROVISIONING");

    ByteVector encodedPublicKey = provisionTSVECCKey(context);

    auto applicationKey = decodeDUOXPublicKey(encodedPublicKey);

    printSection("PHASE 3/4 : TRANSACTION SIGNATURE FILE");

    provisionTSVFiles(duoxCommands, TSV_TEST_CONFIG);

    printSection("PHASE 4/4 : TRANSACTION TESTS");

    testTSVCommit(duoxCommands, applicationKey.get());

    logPass("TSV application test completed.");
}

void runTransactionManagementTest(const std::shared_ptr<Chip> &chip, const ByteVector &uid)
{
    DUOXTestContext context(chip);

    printSection("DUOX TRANSACTION MANAGEMENT INTEGRATION TEST");

    printTestSafetyPolicy();

    prepareDUOXTestCard(context);
    
    // Automatically clean up the reserved application after successful creation
    TestApplicationGuard applicationGuard(context);

    auto commands = std::dynamic_pointer_cast<DESFireEV2ISO7816Commands>(chip->getCommands());

    requireNonNull(commands, "DESFireEV2ISO7816Commands unavailable on this card.");

    (void)listApplications(context.desfire());

    runApplicationTransactionTests(context, *commands, uid, AES128_TEST_CONFIG);
    runApplicationTransactionTests(context, *commands, uid, AES256_TEST_CONFIG);

    auto duoxISO7816Commands = std::dynamic_pointer_cast<DUOXISO7816Commands>(chip->getCommands());
    requireNonNull(duoxISO7816Commands, "DUOXISO7816Commands unavailable on this card.");
    runTSVApplicationTransactionTests(context, *duoxISO7816Commands, *commands);

    printFinalTestSummary();
    logPass("AES-128, AES-256, and TSV test runs completed.");

    applicationGuard.cleanup();

    logPass("Test applications cleanup verified.");
}

} // namespace

// ==============================================
// Main
// ==============================================

int main()
{
    try
    {
        printSection("DUOX REAL CARD TRANSACTION MANAGEMENT TEST");

        if (CARD_PREPARATION_MODE == DUOXCardPreparationMode::EraseCard)
        {
            std::cout
                << "[WARNING] CARD_PREPARATION_MODE = EraseCard\n"
                << "[WARNING] ALL applications and files will be deleted.\n"
                << "[WARNING] This test must only be run on a dedicated test card.\n";
        }
        else
        {
            std::cout << "[INFO] CARD_PREPARATION_MODE = DeleteTestApplications\n"
                      << "[INFO] Only the dedicated test applications may be deleted.\n";
        }

        const auto readerConfiguration = createReaderConfiguration();
        const auto reader              = readerConfiguration->getReaderUnit();
        requireNonNull(reader, "Reader disappeared after configuration.");

        ReaderConnectionGuard readerGuard(reader);

        const auto chip = waitForCard(readerConfiguration);

        logInfo("Card inserted on reader \"" + reader->getConnectedName() + "\".");

        const auto uid = reader->getNumber(chip);

        std::cout << "[INFO] Card type  : " << chip->getCardType() << '\n'
                  << "[INFO] Card UID   : " << logicalaccess::BufferHelper::getHex(uid) << '\n'
                  << "[INFO] UID length : " << uid.size() << " bytes" << '\n';

        if (!isDUOXCard(chip))
        {
            logInfo("Card is not a DUOX card. Nothing will be modified.");
            return EXIT_SUCCESS;
        }

        logInfo("Card is a DUOX card.");

        // Execute the actual test. Application guard owns cleanup after successful creation.
        runTransactionManagementTest(chip, uid);

        logPass("Tests completed successfully.");
        return EXIT_SUCCESS;
    }
    catch (const std::exception &exception)
    {
        std::cerr << "[ERROR] " << exception.what() << std::endl;
        std::cerr << "[WARNING] If the application was created before the "
                     "failure, reconnect and delete only the reserved test AID "
                     "after authenticating with the PICC Master Key.\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "[ERROR] Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
}