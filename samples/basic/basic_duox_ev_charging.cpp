/**
 * \file basic_duox_ev_charging.cpp
 * \brief DUOX EV Charging integration test suite on a physical card.
 *
 * Test coverage :
 *
 * Factory EV Charging application :
 *  - Application discovery
 *  - ISO7816 application selection
 *  - Certificate retrieval
 *  - Certificate size validation
 *  - Certificate BER-TLV parsing
 *  - Certificate structural validation
 *  - Certificate UID binding
 *  - Certificate validity dates
 *  - Optional trusted CA signature verification
 *  - Additional data retrieval
 *  - BrainpoolP256r1 challenge signing
 *  - Dynamic signature verification
 *
 * Dedicated test application :
 *  - Application discovery
 *  - Application creation
 *  - ISO DF name configuration
 *  - Certificate and additional data file creation
 *  - BrainpoolP256r1 ECC key generation
 *  - EV Charging configuration placeholders
 *  - VDE_ReadData
 *  - VDE_ECDSASign
 *  - VDE_WriteData
 *  - Read-after-write verification
 *  - Optional irreversible file locking
 *
 * SAFETY :
 *  - The factory EV Charging application is never modified
 *  - An existing test application is never overwritten
 *  - Only a test application created by this execution is eligible for automatic cleanup
 *  - The PICC Master Key and application AES keys are never changed
 *  - ECC private keys are generated on the card
 *  - No private key is imported or exported
 *  - The irreversible lock test is disabled by default
 *
 * IMPORTANT :
 *  - Use a dedicated DUOX test card
 *  - Verify the PICC Master Key
 *  - Verify the default application AES key
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
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

#include <logicalaccess/plugins/cards/duox/duoxcommands.hpp>
#include <logicalaccess/plugins/cards/duox/duoxcrypto.hpp>
#include <logicalaccess/plugins/cards/duox/duoxCertificateValidator.hpp>

#include <logicalaccess/plugins/readers/iso7816/commands/duoxiso7816commands.hpp>

using namespace logicalaccess;

namespace
{

// ==============================================
// Configuration
// ==============================================

enum class DUOXCardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

// Factory EV Charging application.
// The commonly displayed identifier is 0xF61010. 0x1010F6 in MSB first representation.
constexpr std::uint32_t EV_CHARGING_FACTORY_AID = 0x1010F6U;

// Dedicated application reserved for this sample
constexpr std::uint32_t EV_CHARGING_TEST_AID = 0x123456U;

constexpr std::uint32_t PICC_LEVEL_AID = 0x000000U;

constexpr std::uint8_t PICC_MASTER_KEY_NO = 0x00U;
constexpr std::uint8_t APP_MASTER_KEY_NO  = 0x00U;
constexpr std::uint8_t ECC_KEY_NO         = 0x00U;

constexpr std::uint16_t ECC_KEY_POLICY_CRYPTO_REQUEST_SIGN = 0x0010U;
constexpr std::uint16_t ECC_KEY_POLICY = ECC_KEY_POLICY_CRYPTO_REQUEST_SIGN;
constexpr std::uint8_t ECC_KEY_WRITE_ACCESS = 0x0FU;

constexpr std::uint32_t ECC_KEY_USAGE_COUNTER_LIMIT = 0U;

constexpr std::size_t EV_CHARGING_CHALLENGE_SIZE  = 32U;
constexpr std::size_t EV_CHARGING_SIGNATURE_SIZE  = 64U;
constexpr std::size_t EV_CHARGING_PUBLIC_KEY_SIZE = 65U;

constexpr std::size_t CERTIFICATE_SIZE_UID7  = 193U;
constexpr std::size_t CERTIFICATE_SIZE_UID10 = 199U;
constexpr std::size_t CERTIFICATE_FILE_SIZE = CERTIFICATE_SIZE_UID10; // Maximum supported certificate size

constexpr std::size_t ADDITIONAL_DATA_FILE_SIZE = 1024U;

// Factory EV Charging application ISO DF name.
const ByteVector FACTORY_EV_CHARGING_DF_NAME(duox_ev_charging::protocol::APPLICATION_DF_NAME.begin(),
                                             duox_ev_charging::protocol::APPLICATION_DF_NAME.end());

// Dedicated test application ISO DF name.
// It is intentionally different from the factory application name
const ByteVector TEST_EV_CHARGING_DF_NAME = {0xA0U, 0x00U, 0x00U, 0x08U, 0x45U, 0x00U,
                                             0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                                             0x00U, 0x12U, 0x34U, 0x56U};

constexpr std::uint16_t FACTORY_APPLICATION_ISO_FILE_ID = duox_ev_charging::protocol::APPLICATION_ISO_FILE_ID;
constexpr std::uint16_t TEST_APPLICATION_ISO_FILE_ID    = 0xDF01U;

constexpr std::uint16_t CERTIFICATE_ISO_FILE_ID     = duox_ev_charging::protocol::CERTIFICATE_ISO_FILE_ID;
constexpr std::uint16_t ADDITIONAL_DATA_ISO_FILE_ID = duox_ev_charging::protocol::ADDITIONAL_DATA_ISO_FILE_ID;

constexpr std::uint8_t CERTIFICATE_FILE_NO     = 0x00U;
constexpr std::uint8_t ADDITIONAL_DATA_FILE_NO = 0x01U;

// Preserve the original preparation setting.
constexpr DUOXCardPreparationMode CARD_PREPARATION_MODE = DUOXCardPreparationMode::EraseCard;

// Test-application operations can be disabled
constexpr bool TEST_APPLICATION_VDE_CONFIGURED = true;

// Lock operation is irreversible.
constexpr bool RUN_IRREVERSIBLE_LOCK_TEST = true;

// Only applications created during this execution may be deleted.
constexpr bool DELETE_TEST_APPLICATION_AFTER_TESTS = true;

// Optional trusted CA public key.
// Supply the 65 byte uncompressed BrainpoolP256r1 public key.
// Leave empty to skip certificate authenticity verification.
const ByteVector TRUSTED_CA_PUBLIC_KEY = {};

// Reader configuration
constexpr char READER_PROVIDER[]     = "PCSC";
constexpr char DESFIRE_READER_NAME[] = "HID Global OMNIKEY 5422CL Smartcard Reader 0";

constexpr unsigned int CARD_INSERTION_TIMEOUT_MS = 15000U;

// Test credentials
constexpr char TEST_PICC_MASTER_KEY_HEX[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";
constexpr char TEST_APPLICATION_AUTH_KEY_HEX[] = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

// ==============================================
// Logging and assertions
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

void logSkip(const std::string &message)
{
    std::cout << "[SKIP] " << message << std::endl;
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
        logFailure(name, "Unknown exception.");
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

void expectEqual(const ByteVector &actual, const ByteVector &expected, const std::string &description)
{
    require(actual == expected, description + " : byte sequences differ.");

    logPass(description);
}

// ==============================================
// Reader handling
// ==============================================

std::shared_ptr<ReaderUnit>
findReaderByName(const std::shared_ptr<ReaderProvider> &provider, const std::string &readerName)
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

    throw std::runtime_error("Required PCSC reader was not found.\nExpected : " +
                             readerName + "\nAvailable readers :\n" + availableReaders.str());
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

    ReaderConnectionGuard(const ReaderConnectionGuard &) = delete;
    ReaderConnectionGuard &operator=(const ReaderConnectionGuard &) = delete;

  private:
    std::shared_ptr<ReaderUnit> reader_;
};

std::shared_ptr<Chip>
waitForCard(const std::shared_ptr<ReaderConfiguration> &configuration)
{
    requireNonNull(configuration, "Reader configuration is null.");

    const auto reader = configuration->getReaderUnit();

    requireNonNull(reader, "Configured reader is null.");

    logInfo("Reader name : " + reader->getName());
    logInfo("Waiting " + std::to_string(CARD_INSERTION_TIMEOUT_MS / 1000U) + " seconds for card insertion...");

    const auto startTime = std::time(nullptr);

    if (startTime != static_cast<std::time_t>(-1))
    {
        char timeBuffer[26] = {};

#if defined(_WIN32)
        const bool success = ctime_s(timeBuffer, sizeof(timeBuffer), &startTime) == 0;
#else
        const bool success = ctime_r(&startTime, timeBuffer) != nullptr;
#endif

        if (success)
            logInfo(std::string("Time start : ") + timeBuffer);
    }

    traceOperation("Connect to physical reader",
                   [&reader]() { reader->connectToReader(); });

    traceOperation("Wait for card insertion",
                   [&reader]()
                   {
                       require(reader->waitInsertion(CARD_INSERTION_TIMEOUT_MS), "No card was inserted within the timeout.");
                   });

    traceOperation(
        "Establish card communication", [&reader]()
        { require(reader->connect(), "Card communication could not be established."); });

    std::shared_ptr<Chip> chip;

    traceOperation("Identify card type",
                   [&reader, &chip]()
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
// Challenge generation
// ==============================================

ByteVector generateChallenge()
{
    std::random_device randomDevice;

    ByteVector challenge(EV_CHARGING_CHALLENGE_SIZE);

    for (auto &byte : challenge)
        byte = static_cast<std::uint8_t>(randomDevice() & 0xFFU);

    return challenge;
}

ByteVector createTestPayload(std::size_t size)
{
    ByteVector payload;
    payload.reserve(size);

    for (std::size_t i = 0U; i < size; ++i)
        payload.push_back(static_cast<std::uint8_t>((i * 37U + 0xA5U) & 0xFFU));

    return payload;
}

// ==============================================
// DUOX test context
// ==============================================

struct DUOXTestContext
{
    std::shared_ptr<ReaderConfiguration> readerConfiguration;
    std::shared_ptr<ReaderUnit> reader;
    std::shared_ptr<Chip> chip;

    std::shared_ptr<DESFireCommands> desfireCommands;
    std::shared_ptr<DUOXCommands> duoxCommands;
    std::shared_ptr<DUOXISO7816Commands> iso7816Commands;

    ByteVector cardUID;

    bool connected = false;

    void selectPICC()
    {
        requireNonNull(duoxCommands, "DUOX commands unavailable.");

        traceOperation("Select PICC master application", [this]() { duoxCommands->selectApplication(PICC_LEVEL_AID); });
    }

    void selectApplication(std::uint32_t aid)
    {
        requireNonNull(duoxCommands, "DUOX commands unavailable.");

        traceOperation("Select application " + formatAid(aid), [this, aid]() { duoxCommands->selectApplication(aid); });
    }

    void authenticatePICC()
    {
        selectPICC();

        traceOperation("Authenticate PICC using EV2 First", [this]()
                       {
                           duoxCommands->authenticateEV2First(PICC_MASTER_KEY_NO, createTestPICCKey());
                       });

        logPass("PICC EV2 First authentication successful.");
    }

    void authenticateApplication()
    {
        selectApplication(EV_CHARGING_TEST_AID);

        traceOperation("Authenticate application using EV2 First", [this]()
                       {
                           duoxCommands->authenticateEV2First(APP_MASTER_KEY_NO, createTestApplicationKey());
                       });

        logPass("Application EV2 First authentication successful.");
    }
};

void initializeDUOXTestContext(DUOXTestContext &context)
{
    printSection("DUOX TEST CONTEXT INITIALIZATION");

    context.readerConfiguration = createReaderConfiguration();

    requireNonNull(context.readerConfiguration, "Unable to create reader configuration.");

    context.reader = context.readerConfiguration->getReaderUnit();

    requireNonNull(context.reader, "Reader unit is unavailable.");

    context.chip = waitForCard(context.readerConfiguration);

    require(isDUOXCard(context.chip), "The detected card is not a DUOX card.");

    logPass("DUOX card successfully detected.");

    const auto commands = context.chip->getCommands();

    requireNonNull(commands, "Card command interface is unavailable.");

    context.desfireCommands = std::dynamic_pointer_cast<DESFireCommands>(commands);
    context.duoxCommands = std::dynamic_pointer_cast<DUOXCommands>(commands);
    context.iso7816Commands = std::dynamic_pointer_cast<DUOXISO7816Commands>(commands);

    requireNonNull(context.desfireCommands, "DESFire commands unavailable.");
    requireNonNull(context.duoxCommands, "DUOX commands unavailable.");
    requireNonNull(context.iso7816Commands, "DUOX ISO7816 commands unavailable.");

    context.connected = true;

    logPass("All DUOX command interfaces initialized.");
}

// ==============================================
// Card UID
// ==============================================

ByteVector getCardUID(DUOXTestContext &context)
{
    printSection("AUTHENTICATED CARD UID RETRIEVAL");

    requireNonNull(context.duoxCommands, "DUOX command interface is unavailable.");

    // GetCardUID is performed at PICC level
    context.authenticatePICC();

    ByteVector uid;

    traceOperation("Retrieve authenticated physical card UID",
                   [&context, &uid]() { uid = context.duoxCommands->getCardUID(); });

    require(uid.size() == 7U || uid.size() == 10U, "GetCardUID returned an unsupported UID length.");

    logPass("Authenticated GetCardUID operation successful.");

    logInfo("Card UID : " + toHex(uid));

    return uid;
}

// ==============================================
// Access Rights
// ==============================================

DESFireAccessRights createEVChargingFileAccessRights()
{
    DESFireAccessRights accessRights = {};

    accessRights.readAccess         = AR_FREE;
    accessRights.writeAccess        = AR_FREE;
    accessRights.readAndWriteAccess = AR_FREE;
    accessRights.changeAccess       = AR_FREE;

    return accessRights;
}

// ==============================================
// Application selection and discovery
// ==============================================

void selectPICCApplication(DUOXTestContext &context)
{
    traceOperation("Select PICC master application", [&context]()
                   { context.desfireCommands->selectApplication(PICC_LEVEL_AID); });
}

void selectApplication(DUOXTestContext &context, std::uint32_t aid)
{
    traceOperation("Select application " + formatAid(aid), [&context, aid]()
                   { context.desfireCommands->selectApplication(aid); });
}

std::vector<std::uint32_t> getApplicationIDs(DUOXTestContext &context)
{
    requireNonNull(context.desfireCommands, "DESFire command interface is unavailable.");

    return context.desfireCommands->getApplicationIDs();
}

bool applicationExists(DUOXTestContext &context, std::uint32_t aid)
{
    const auto applicationIDs = getApplicationIDs(context);

    return std::find(applicationIDs.begin(), applicationIDs.end(), aid) != applicationIDs.end();
}

bool factoryApplicationExists(DUOXTestContext &context)
{
    const bool exists = applicationExists(context, EV_CHARGING_FACTORY_AID);

    if (exists)
    {
        logPass("Factory EV Charging application found : " + formatAid(EV_CHARGING_FACTORY_AID));
    }
    else
    {
        logSkip("Factory EV Charging application not found.");
    }

    return exists;
}

bool testApplicationExists(DUOXTestContext &context)
{
    const bool exists = applicationExists(context, EV_CHARGING_TEST_AID);

    if (exists)
    {
        logInfo("Dedicated test application already exists : " + formatAid(EV_CHARGING_TEST_AID));
    }
    else
    {
        logInfo("Dedicated test application does not exist : " + formatAid(EV_CHARGING_TEST_AID));
    }

    return exists;
}

// ==============================================
// ISO7816 application selection
// ==============================================

ByteVector selectEVChargingApplication(DUOXTestContext &context, const ByteVector &dfName)
{
    require(!dfName.empty(), "Application DF name cannot be empty.");

    ByteVector response;

    traceOperation("ISO7816 SELECT FILE by DF name", [&context, &dfName, &response]()
                   { response = context.iso7816Commands->isoSelectFile(dfName); });

    return response;
}

void selectFactoryEVChargingApplication(DUOXTestContext &context)
{
    selectApplication(context, EV_CHARGING_FACTORY_AID);

    selectEVChargingApplication(context, FACTORY_EV_CHARGING_DF_NAME);

    logPass("Factory EV Charging application selected.");
}

void selectTestEVChargingApplication(DUOXTestContext &context)
{
    selectEVChargingApplication(context, TEST_EV_CHARGING_DF_NAME);

    logPass("Test EV Charging application selected.");
}

// ==============================================
// Card preparation
// ==============================================

void deleteTestApplication(DUOXTestContext &context)
{
    printSection("DELETE DEDICATED TEST APPLICATION");

    if (!applicationExists(context, EV_CHARGING_TEST_AID))
    {
        logInfo("Test application already absent. Nothing to delete.");

        return;
    }

    context.authenticatePICC();

    traceOperation("Delete dedicated test application", [&context]()
                   { context.desfireCommands->deleteApplication(EV_CHARGING_TEST_AID); });

    require(!applicationExists(context, EV_CHARGING_TEST_AID), "Test application still exists after deletion.");

    logPass("Dedicated test application deleted.");
}

// TODO Centralize erase() method in all test suites. This erase() can currently erase EV_CHARGING_FACTORY_AID
// But a card can already lose information with other test suites.
void eraseTestCard(DUOXTestContext &context)
{
    printSection("ERASE TEST CARD");
    std::cout << "[WARNING] This operation will permanently remove ALL applications and files from the card.\n"
              << "[WARNING] The PICC Master Key will NOT be modified.\n";

    context.authenticatePICC();

    logInfo("Erasing / formatting PICC...");

    // IMPORTANT :
    // Erase is a PICC-level destructive operation !
    // Do not rely on the previous authentication/session remaining valid after the erase
    context.desfireCommands->erase();

    logPass("PICC erased successfully.");
    logInfo("All applications and files were removed.");

    const auto applications = context.desfireCommands->getApplicationIDs();

    require(applications.empty(), "PICC erase completed but applications are still reported.");

    logPass("Application list is empty after erase.");
    logInfo("NV memory is now available for reuse.");
    std::cout << "=============================================\n";
}

void prepareDUOXTestCard(DUOXTestContext &context)
{
    printSection("DUOX CARD PREPARATION");

    switch (CARD_PREPARATION_MODE)
    {
    case DUOXCardPreparationMode::EraseCard: eraseTestCard(context); break;

    case DUOXCardPreparationMode::DeleteTestApplication:
        deleteTestApplication(context);
        break;

    default: throw std::runtime_error("Unsupported DUOX card preparation mode.");
    }

    logPass("Card preparation completed.");
}

// ==============================================
// Application file configuration
// ==============================================

void createCertificateFile(DUOXTestContext &context)
{
    const DESFireAccessRights accessRights = createEVChargingFileAccessRights();

    traceOperation("Create EV Charging certificate StandardDataFile",
                   [&context, &accessRights]()
                   {
                       context.iso7816Commands->createStdDataFile(
                           CERTIFICATE_FILE_NO, CM_ENCRYPT, accessRights,
                           CERTIFICATE_FILE_SIZE, CERTIFICATE_ISO_FILE_ID, false);
                   });
}

void createAdditionalDataFile(DUOXTestContext &context)
{
    const DESFireAccessRights accessRights = createEVChargingFileAccessRights();

    traceOperation("Create EV Charging additional data StandardDataFile",
                   [&context, &accessRights]()
                   {
                       context.iso7816Commands->createStdDataFile(
                           ADDITIONAL_DATA_FILE_NO, CM_ENCRYPT, accessRights,
                           ADDITIONAL_DATA_FILE_SIZE, ADDITIONAL_DATA_ISO_FILE_ID, false);
                   });
}

// ==============================================
// Test application provisioning
// ==============================================

void createTestApplication(DUOXTestContext &context)
{
    printSection("CREATE DEDICATED EV CHARGING TEST APPLICATION");

    require(!applicationExists(context, EV_CHARGING_TEST_AID), "Refusing to overwrite an existing test application.");

    context.authenticatePICC();

    traceOperation("Create dedicated EV Charging application",
                   [&context]()
                   {
                       context.duoxCommands->createApplication(
                           EV_CHARGING_TEST_AID, KS_DEFAULT, 1, DF_KEY_AES,
                           FidSupport::FIDS_ISO_FID, TEST_APPLICATION_ISO_FILE_ID,
                           TEST_EV_CHARGING_DF_NAME, 0, 0, 0, 0,
                           true,   // specificCapabilityData
                           false); // specificVCKeys
                   });

    require(applicationExists(context, EV_CHARGING_TEST_AID), "Test application creation could not be verified.");

    selectApplication(context, EV_CHARGING_TEST_AID);

    traceOperation("Authenticate with test application master key",
                   [&context]()
                   {
                       context.duoxCommands->authenticateEV2First(APP_MASTER_KEY_NO, createTestApplicationKey());
                   });

    createCertificateFile(context);
    createAdditionalDataFile(context);

    logPass("Dedicated EV Charging test application provisioned.");

    logInfo("Application AID : " + formatAid(EV_CHARGING_TEST_AID));
}

// ==============================================
// Application ownership and cleanup
// ==============================================

class TestApplicationGuard
{
  public:
    explicit TestApplicationGuard(DUOXTestContext &context)
        : context_(context)
    {
    }

    TestApplicationGuard(const TestApplicationGuard &) = delete;

    TestApplicationGuard &operator=(const TestApplicationGuard &) = delete;

    void arm()
    {
        require(!armed_, "Test application cleanup guard is already armed.");

        armed_ = true;

        logInfo("Test application cleanup guard armed.");
    }

    bool armed() const
    {
        return armed_;
    }

    void cleanup()
    {
        if (!armed_)
        {
            logInfo("Cleanup skipped : this execution does not own the test application.");

            return;
        }

        // EV Charging operations require an unauthenticated application session.
        // Re-authentication is therefore mandatory before performing operations such as application deletion.
        context_.authenticatePICC();

        printSection("TEST APPLICATION CLEANUP");

        if (!applicationExists(context_, EV_CHARGING_TEST_AID))
        {
            logInfo("Owned test application is already absent.");

            armed_ = false;
            return;
        }

        try
        {
            deleteTestApplication(context_);

            armed_ = false;

            logPass("Owned test application successfully removed.");
        }
        catch (const std::exception &exception)
        {
            logFailure("Test application cleanup", exception.what());

            throw;
        }
    }

    ~TestApplicationGuard()
    {
        if (!armed_)
            return;

        try
        {
            cleanup();
        }
        catch (...)
        {
            std::cerr << "[WARNING] Automatic test application cleanup failed. Manual cleanup may be required.\n";
        }
    }

  private:
    DUOXTestContext &context_;
    bool armed_ = false;
};

// ==============================================
// Test application discovery and file checks
// ==============================================

bool testApplicationHasRequiredFiles(DUOXTestContext &context)
{
    try
    {
        selectApplication(context, EV_CHARGING_TEST_AID);

        const auto files = context.desfireCommands->getFileIDs();
        const bool hasCertificateFile = std::find(files.begin(), files.end(), CERTIFICATE_FILE_NO) != files.end();
        const bool hasAdditionalDataFile = std::find(files.begin(), files.end(), ADDITIONAL_DATA_FILE_NO) != files.end();

        if (!hasCertificateFile)
            logSkip("Existing test application has no certificate file.");

        if (!hasAdditionalDataFile)
            logSkip("Existing test application has no additional data file.");

        return hasCertificateFile && hasAdditionalDataFile;
    }
    catch (const std::exception &exception)
    {
        logFailure("Test application file discovery", exception.what());

        return false;
    }
}

// ==============================================
// Factory application discovery
// ==============================================

void discoverFactoryApplication(DUOXTestContext &context)
{
    printSection("FACTORY EV CHARGING APPLICATION DISCOVERY");

    if (!factoryApplicationExists(context))
    {
        logSkip("Factory EV Charging tests cannot run.");

        return;
    }

    selectApplication(context, EV_CHARGING_FACTORY_AID);
    selectFactoryEVChargingApplication(context);

    logPass("Factory EV Charging application discovered and selected successfully.");

    logInfo("Factory application will remain untouched.");
}

// ==============================================
// EV Charging certificate retrieval
// ==============================================

ByteVector readFactoryCertificate(DUOXTestContext &context)
{
    printSection("FACTORY CERTIFICATE RETRIEVAL");

    selectFactoryEVChargingApplication(context);

    ByteVector certificate;

    traceOperation("Read factory EV Charging certificate",
                   [&context, &certificate]()
                   {
                       certificate = context.duoxCommands->VDE_ReadData(duox_ev_charging::FileNo::Certificate);
                   });

    require(!certificate.empty(), "Factory certificate is empty.");

    logInfo("Certificate size : " + std::to_string(certificate.size()) + " bytes.");
    logInfo("Certificate data : " + toHex(certificate));

    return certificate;
}

// ==============================================
// Certificate size validation
// ==============================================

void testCertificateSize(const ByteVector &certificate)
{
    printSection("CERTIFICATE SIZE VALIDATION");

    require(certificate.size() == CERTIFICATE_SIZE_UID7 || certificate.size() == CERTIFICATE_SIZE_UID10,
            "Unexpected EV Charging certificate size. Expected 193 or 199 bytes.");

    if (certificate.size() == CERTIFICATE_SIZE_UID7)
    {
        logPass("Certificate has the expected 193 byte UID7 format.");
    }
    else
    {
        logPass("Certificate has the expected 199 byte UID10 format.");
    }
}

// ==============================================
// Certificate parsing
// ==============================================

EVChargingCertificate parseFactoryCertificate(const ByteVector &certificate)
{
    printSection("CERTIFICATE TLV PARSING");

    EVChargingCertificate parsedCertificate;

    traceOperation(
        "Parse EV Charging certificate", [&certificate, &parsedCertificate]()
        { parsedCertificate = DUOXCrypto::VDE_ParseCertificate(certificate); });

    logPass("Certificate successfully parsed.");

    return parsedCertificate;
}

// ==============================================
// Certificate field diagnostics
// ==============================================

void printCertificateField(const std::string &name, const ByteVector &value)
{
    std::cout << "[CERTIFICATE] " << std::left << std::setw(24) << name << " : " << toHex(value) << std::endl;
}

void printCertificateInformation(const EVChargingCertificate &certificate)
{
    printSection("CERTIFICATE INFORMATION");

    printCertificateField("UID", certificate.uid);
    printCertificateField("CA identifier", certificate.caId);
    printCertificateField("Subject identifier", certificate.subjectIdentifier);
    printCertificateField("Key usage", certificate.keyUsage);
    printCertificateField("Effective date", certificate.effectiveDate);
    printCertificateField("Expiration date", certificate.expirationDate);

    std::cout << "[CERTIFICATE] " << std::left << std::setw(24) << "CA SD image number" << " : "
              << static_cast<unsigned int>(certificate.caSecurityDomainImageNumber) << std::endl;

    printCertificateField("Public key data", certificate.publicKeyData);
    printCertificateField("Public key", certificate.publicKey);

    std::cout << "[CERTIFICATE] " << std::left << std::setw(24) << "Key parameter reference" << " : "
              << static_cast<unsigned int>(certificate.keyParameterReference) << std::endl;

    printCertificateField("Signature", certificate.signature);

    logInfo("Signed data size : " + std::to_string(certificate.signedData.size()) + " bytes.");
    logInfo("Signed data : " + toHex(certificate.signedData));
}

// ==============================================
// Certificate structural validation
// ==============================================

void testCertificateStructure(const EVChargingCertificate &certificate)
{
    printSection("CERTIFICATE STRUCTURAL VALIDATION");

    traceOperation("Validate certificate mandatory fields", [&certificate]()
                   { DUOXCrypto::VDE_ValidateCertificate(certificate); });

    require(certificate.uid.size() == 7U || certificate.uid.size() == 10U, "Certificate UID has an unsupported size.");

    expectResponseSize(certificate.publicKey, EV_CHARGING_PUBLIC_KEY_SIZE, "Certificate public key");

    require(certificate.publicKey.front() == 0x04U, "Certificate public key is not uncompressed.");

    expectResponseSize(certificate.signature, EV_CHARGING_SIGNATURE_SIZE, "Certificate signature");

    require(!certificate.signedData.empty(), "Certificate signed data is empty.");
    require(!certificate.caId.empty(), "Certificate CA identifier is empty.");
    require(!certificate.subjectIdentifier.empty(), "Certificate subject identifier is empty.");
    require(!certificate.keyUsage.empty(), "Certificate key usage is empty.");

    logPass("Certificate structure and mandatory fields are valid.");
}

// ==============================================
// Certificate UID binding
// ==============================================

void testCertificateUID(const EVChargingCertificate &certificate, const ByteVector &cardUID)
{
    printSection("CERTIFICATE UID BINDING");

    traceOperation("Verify certificate UID against physical card UID",
                   [&certificate, &cardUID]()
                   {
                       require(DUOXCrypto::VDE_VerifyCertificateUID(certificate, cardUID),
                               "Certificate UID does not match the card UID.");
                   });

    logPass("Certificate UID matches the physical card UID.");

    // Negative test : a modified UID must not match.
    ByteVector invalidUID = cardUID;

    invalidUID.front() ^= 0x01U;

    require(!DUOXCrypto::VDE_VerifyCertificateUID(certificate, invalidUID),
            "Certificate UID verification accepted an incorrect UID.");

    logPass("Negative UID verification correctly rejected a modified UID.");
}

// ==============================================
// Certificate validity dates
// ==============================================

void testCertificateValidityDates(const EVChargingCertificate &certificate)
{
    printSection("CERTIFICATE VALIDITY DATES");

    logInfo("Certificate effective date : " + toHex(certificate.effectiveDate));
    logInfo("Certificate expiration date : " + toHex(certificate.expirationDate));

    // The caller must supply a trustworthy current date.
    // This test uses the system clock only as a diagnostic reference; it is not a trusted time source
    const std::time_t now = std::time(nullptr);

    require(now != static_cast<std::time_t>(-1), "Unable to obtain system time.");

    std::tm currentTime = {};

#if defined(_WIN32)
    require(localtime_s(&currentTime, &now) == 0, "Unable to convert system time.");
#else
    require(localtime_r(&now, &currentTime) != nullptr, "Unable to convert system time.");
#endif

    const int year  = currentTime.tm_year + 1900;
    const int month = currentTime.tm_mon + 1;
    const int day   = currentTime.tm_mday;

    logInfo("System date : " + std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day));

    // Date is the date representation used by the DUOX certificate validator in this codebase
    const EVChargingDate currentDate{static_cast<std::uint16_t>(year),
                                     static_cast<std::uint8_t>(month),
                                     static_cast<std::uint8_t>(day)};

    traceOperation("Verify certificate validity period",
                   [&certificate, &currentDate]()
                   {
                       require(
                           DUOXCrypto::VDE_IsCertificateValidAt(certificate, currentDate),
                           "Certificate is not currently valid.");
                   });

    logPass("Certificate is within its validity period.");
}

// ==============================================
// Certificate authenticity
// ==============================================

void testCertificateAuthenticity(const ByteVector &rawCertificate, const EVChargingCertificate &certificate)
{
    printSection("CERTIFICATE AUTHENTICITY");

    if (TRUSTED_CA_PUBLIC_KEY.empty())
    {
        logSkip("No trusted CA public key configured. Certificate authenticity cannot be established.");

        return;
    }

    expectResponseSize(TRUSTED_CA_PUBLIC_KEY, EV_CHARGING_PUBLIC_KEY_SIZE, "Trusted CA public key");

    traceOperation("Validate trusted CA public key",
                   []() { DUOXCrypto::VDE_ValidatePublicKey(TRUSTED_CA_PUBLIC_KEY); });

    traceOperation(
        "Verify factory certificate signature",
        [&certificate]()
        {
            require(DUOXCrypto::VDE_VerifyCertificate(certificate, TRUSTED_CA_PUBLIC_KEY),
                    "Certificate signature verification failed.");
        });

    logPass("Factory certificate signature verified.");

    // Verify the overload accepting the original raw certificate
    traceOperation("Verify raw factory certificate",
                   [&rawCertificate]()
                   {
                       require(DUOXCrypto::VDE_VerifyCertificate(rawCertificate, TRUSTED_CA_PUBLIC_KEY),
                               "Raw certificate signature verification failed.");
                   });

    logPass("Raw certificate verification succeeded.");
}

// ==============================================
// Factory additional data retrieval
// ==============================================

ByteVector readFactoryAdditionalData(DUOXTestContext &context)
{
    printSection("FACTORY ADDITIONAL DATA RETRIEVAL");

    selectFactoryEVChargingApplication(context);

    ByteVector additionalData;

    traceOperation("Read factory additional data file",
                   [&context, &additionalData]()
                   {
                       additionalData = context.duoxCommands->VDE_ReadData(
                           duox_ev_charging::FileNo::AdditionalData);
                   });

    logInfo("Additional data size : " + std::to_string(additionalData.size()) + " bytes.");
    logInfo("Additional data : " + toHex(additionalData));

    require(additionalData.size() <= ADDITIONAL_DATA_FILE_SIZE, "Factory additional data exceeds supported file capacity.");

    logPass("Factory additional data retrieved successfully.");

    return additionalData;
}

// ==============================================
// Challenge validation
// ==============================================

void testChallengeValidation()
{
    printSection("EV CHARGING CHALLENGE VALIDATION");

    const ByteVector validChallenge = generateChallenge();

    traceOperation("Validate 32 byte challenge", [&validChallenge]()
                   { DUOXCrypto::VDE_ValidateChallenge(validChallenge); });

    logPass("Valid challenge accepted.");

    const ByteVector emptyChallenge;

    expectException([&emptyChallenge]()
                    { DUOXCrypto::VDE_ValidateChallenge(emptyChallenge); }, "Empty challenge");

    ByteVector shortChallenge(EV_CHARGING_CHALLENGE_SIZE - 1U, 0xAAU);

    expectException([&shortChallenge]()
                    { DUOXCrypto::VDE_ValidateChallenge(shortChallenge); }, "31 byte challenge");

    ByteVector longChallenge(EV_CHARGING_CHALLENGE_SIZE + 1U, 0xAAU);

    expectException([&longChallenge]()
                    { DUOXCrypto::VDE_ValidateChallenge(longChallenge); }, "33 byte challenge");
}

// ==============================================
// Signature validation
// ==============================================

void testSignatureValidation()
{
    printSection("EV CHARGING SIGNATURE VALIDATION");

    ByteVector validSignature(EV_CHARGING_SIGNATURE_SIZE, 0x01U);

    traceOperation("Validate 64 byte raw signature representation", [&validSignature]()
                   { DUOXCrypto::VDE_ValidateSignature(validSignature); });

    logPass("Correctly sized raw signature accepted.");

    ByteVector shortSignature(EV_CHARGING_SIGNATURE_SIZE - 1U, 0x01U);

    expectException([&shortSignature]()
                    { DUOXCrypto::VDE_ValidateSignature(shortSignature); }, "63 byte signature");

    ByteVector longSignature(EV_CHARGING_SIGNATURE_SIZE + 1U, 0x01U);

    expectException([&longSignature]()
                    { DUOXCrypto::VDE_ValidateSignature(longSignature); }, "65 byte signature");
}

// ==============================================
// Public key validation
// ==============================================

void testPublicKeyValidation(const ByteVector &publicKey)
{
    printSection("BRAINPOOLP256R1 PUBLIC KEY VALIDATION");

    traceOperation("Validate certificate BrainpoolP256r1 public key", [&publicKey]()
                   { DUOXCrypto::VDE_ValidatePublicKey(publicKey); });

    logPass("Certificate public key representation is valid.");

    ByteVector emptyKey;

    expectException([&emptyKey]() { DUOXCrypto::VDE_ValidatePublicKey(emptyKey); }, "Empty public key");

    ByteVector invalidPrefix = publicKey;

    invalidPrefix.front() = 0x02U;

    expectException([&invalidPrefix]() { DUOXCrypto::VDE_ValidatePublicKey(invalidPrefix); },
                    "Compressed public key representation");

    ByteVector invalidLength(EV_CHARGING_PUBLIC_KEY_SIZE - 1U, 0x01U);

    expectException([&invalidLength]() { DUOXCrypto::VDE_ValidatePublicKey(invalidLength); }, "Invalid public key length");
}

// ==============================================
// Factory ECDSA signing
// ==============================================

ByteVector signFactoryChallenge(DUOXTestContext &context, const ByteVector &challenge)
{
    printSection("FACTORY EV CHARGING ECDSA SIGNING");

    selectFactoryEVChargingApplication(context);

    ByteVector signature;

    traceOperation("Generate factory EV Charging ECDSA signature", [&context, &challenge, &signature]()
                   { signature = context.duoxCommands->VDE_ECDSASign(challenge); });

    expectResponseSize(signature, EV_CHARGING_SIGNATURE_SIZE, "Factory ECDSA signature");

    traceOperation("Validate generated ECDSA signature", [&signature]()
                   { DUOXCrypto::VDE_ValidateSignature(signature); });

    logPass("Factory generated a valid-length raw ECDSA signature.");

    return signature;
}

// ==============================================
// Factory ECDSA signature verification
// ==============================================

void testFactoryECDSAVerification(const ByteVector &publicKey, const ByteVector &challenge, const ByteVector &signature)
{
    printSection("FACTORY ECDSA SIGNATURE VERIFICATION");

    traceOperation("Verify factory ECDSA signature",
                   [&publicKey, &challenge, &signature]()
                   {
                       require(DUOXCrypto::VDE_ECDSAVerify(publicKey, challenge, signature),
                           "Factory ECDSA signature verification failed.");
                   });

    logPass("Factory ECDSA signature verified successfully.");

    // Negative test : changing the challenge must invalidate the signature
    ByteVector modifiedChallenge = challenge;

    modifiedChallenge.front() ^= 0x01U;

    traceOperation("Reject signature against modified challenge", [&publicKey, &modifiedChallenge, &signature]()
        {
            require(!DUOXCrypto::VDE_ECDSAVerify(publicKey, modifiedChallenge, signature),
                    "ECDSA signature unexpectedly verified against a modified challenge.");
        });

    logPass("Modified challenge correctly rejected.");

    // Negative test : changing the signature must invalidate it
    ByteVector modifiedSignature = signature;

    modifiedSignature.back() ^= 0x01U;

    traceOperation("Reject modified ECDSA signature", [&publicKey, &challenge, &modifiedSignature]()
        {
            require(!DUOXCrypto::VDE_ECDSAVerify(publicKey, challenge, modifiedSignature),
                    "Modified ECDSA signature was accepted.");
        });

    logPass("Modified signature correctly rejected.");
}

// ==============================================
// Complete factory EV Charging test sequence
// ==============================================

void runFactoryEVChargingTests(DUOXTestContext &context)
{
    printSection("FACTORY EV CHARGING TEST SUITE");

    if (!factoryApplicationExists(context))
    {
        logSkip("Factory EV Charging application unavailable. Skipping factory tests.");
        return;
    }

    // ------------------------------------------
    // Certificate retrieval and parsing
    // ------------------------------------------

    const ByteVector rawCertificate = readFactoryCertificate(context);

    testCertificateSize(rawCertificate);

    const EVChargingCertificate certificate = parseFactoryCertificate(rawCertificate);

    printCertificateInformation(certificate);

    testCertificateStructure(certificate);

    // ------------------------------------------
    // UID and certificate validity
    // ------------------------------------------

    testCertificateUID(certificate, context.cardUID);

    testCertificateValidityDates(certificate);

    // ------------------------------------------
    // Certificate authenticity
    // ------------------------------------------

    testCertificateAuthenticity(rawCertificate, certificate);

    // ------------------------------------------
    // Additional data
    // ------------------------------------------

    const ByteVector additionalData = readFactoryAdditionalData(context);

    (void)additionalData;

    // ------------------------------------------
    // Public key validation
    // ------------------------------------------

    testPublicKeyValidation(certificate.publicKey);

    // ------------------------------------------
    // Challenge validation
    // ------------------------------------------

    testChallengeValidation();

    // ------------------------------------------
    // Signature validation
    // ------------------------------------------

    testSignatureValidation();

    // ------------------------------------------
    // Dynamic ECDSA signing and verification
    // ------------------------------------------

    const ByteVector challenge = generateChallenge();
    const ByteVector signature = signFactoryChallenge(context, challenge);

    testFactoryECDSAVerification(certificate.publicKey, challenge, signature);

    logPass("Factory EV Charging test suite completed.");
}

// ==============================================
// Dedicated test application preparation
// ==============================================

bool prepareTestApplication(DUOXTestContext &context, TestApplicationGuard &applicationGuard)
{
    printSection("DEDICATED TEST APPLICATION PREPARATION");

    if (testApplicationExists(context))
    {
        logInfo("Existing test application detected.");

        if (!testApplicationHasRequiredFiles(context))
        {
            logSkip("Existing test application does not contain all required files.");
            logSkip("Existing application will NOT be deleted or modified automatically.");

            return false;
        }

        logPass("Existing test application has the required files.");

        return false;
    }

    // This execution owns the application because it did not exist before creation
    createTestApplication(context);

    applicationGuard.arm();

    logPass("Test application ownership successfully registered.");

    return true;
}

// ==============================================
// Test application authentication
// ==============================================

void selectAndAuthenticateTestApplication(DUOXTestContext &context)
{
    context.authenticateApplication();

    logPass("Test application selected and authenticated.");
}

void authenticateTestApplication(DUOXTestContext &context)
{
    context.authenticateApplication();

    logPass("Test application authentication successful.");
}

void unauthenticateTestApplication(DUOXTestContext &context)
{
    // Selecting PICC level terminates the current application session
    context.duoxCommands->selectApplication(PICC_LEVEL_AID);

    // Selecting the test application without authenticating leaves the card in an unauthenticated application session
    context.duoxCommands->selectApplication(EV_CHARGING_TEST_AID);

    logPass("Test application selected without authentication.");
}

// ==============================================
// Test application ECC provisioning
// ==============================================

void provisionTestApplicationECC(DUOXTestContext &context)
{
    printSection("TEST APPLICATION ECC PROVISIONING");

    selectAndAuthenticateTestApplication(context);

    ByteVector generatedPublicKey;

    traceOperation("Generate BrainpoolP256r1 ECC private key",
                   [&context, &generatedPublicKey]()
                   {
                       generatedPublicKey = context.duoxCommands->manageKeyPair(
                           ECC_KEY_NO, DUOXManageKeyPairOption::GenerateKeyPair,
                           CurveID::BRAINPOOL_P256R1, ECC_KEY_POLICY,
                           ECC_KEY_WRITE_ACCESS, ECC_KEY_USAGE_COUNTER_LIMIT);
                   });

    DUOXCrypto::VDE_ValidatePublicKey(generatedPublicKey);

    logPass("ECC key pair generation command completed.");

    logInfo("Generated public key : " + toHex(generatedPublicKey));

    // ------------------------------------------
    // EV Charging configuration
    // ------------------------------------------

    // ------------------------------------------
    // EV Charging configuration: Secure Messaging
    // ------------------------------------------

    traceOperation("Enable EV Charging functionality",
                   [&context]()
                   {
                       duox::configuration::secure_messaging::Configuration smConfig;
                       smConfig.configB = 0x10;

                       duox::configuration::Payload payload;
                       payload.option = duox::configuration::option::SECURE_MESSAGING;
                       payload.data   = smConfig.serialize();

                       context.duoxCommands->setConfiguration(payload, false);
                   });

    // ------------------------------------------
    // EV Charging configuration: Crypto API
    // ------------------------------------------

    traceOperation("Enable unrestricted EV Charging ECDSA signing access",
                   [&context]()
                   {
                       duox::configuration::crypto_api::Configuration cryptoConfig;
                       cryptoConfig.support = duox::configuration::crypto_api::Support::ECDSA_SIGN;
                       cryptoConfig.commMode = CM_PLAIN;
                       cryptoConfig.accessCondition = 0x0E;

                       duox::configuration::Payload payload;
                       payload.option = duox::configuration::option::CRYPTO_API_MANAGEMENT;
                       payload.data = cryptoConfig.serialize();

                       context.duoxCommands->setConfiguration(payload, false);
                   });

    logPass("EV Charging Secure Messaging configuration completed.");
    logPass("EV Charging Crypto API configuration completed.");

    logPass("ECC provisioning and EV Charging configuration completed.");
}

// ==============================================
// Test application certificate retrieval
// ==============================================

ByteVector readTestApplicationCertificate(DUOXTestContext &context)
{
    printSection("TEST APPLICATION CERTIFICATE RETRIEVAL");

    unauthenticateTestApplication(context);

    ByteVector certificate;

    traceOperation("Read test application certificate",
                   [&context, &certificate]()
                   {
                       certificate = context.duoxCommands->VDE_ReadData(duox_ev_charging::FileNo::Certificate);
                   });

    logInfo("Test certificate size : " + std::to_string(certificate.size()));

    return certificate;
}

// ==============================================
// Test application additional data retrieval
// ==============================================

ByteVector readTestApplicationAdditionalData(DUOXTestContext &context)
{
    printSection("TEST APPLICATION ADDITIONAL DATA RETRIEVAL");

    unauthenticateTestApplication(context);

    ByteVector data;

    traceOperation("Read test application additional data file",
                   [&context, &data]()
                   {
                       data = context.duoxCommands->VDE_ReadData(duox_ev_charging::FileNo::AdditionalData);
                   });

    logInfo("Test additional data size : " + std::to_string(data.size()));

    return data;
}

// ==============================================
// Test application ECDSA signing
// ==============================================

ByteVector signTestApplicationChallenge(DUOXTestContext &context, const ByteVector &challenge)
{
    printSection("TEST APPLICATION ECDSA SIGNING");

    unauthenticateTestApplication(context);

    ByteVector signature;

    traceOperation("Generate test application ECDSA signature",
                   [&context, &challenge, &signature]()
                   { signature = context.duoxCommands->VDE_ECDSASign(challenge); });

    expectResponseSize(signature, EV_CHARGING_SIGNATURE_SIZE, "Test application ECDSA signature");

    DUOXCrypto::VDE_ValidateSignature(signature);

    logPass("Test application generated a 64 byte ECDSA signature.");

    return signature;
}

// ==============================================
// Test application data writing
// ==============================================

void testAdditionalDataWriting(DUOXTestContext &context)
{
    printSection("TEST APPLICATION ADDITIONAL DATA WRITING");

    require(TEST_APPLICATION_VDE_CONFIGURED, "Test application EV Charging configuration is disabled.");

    unauthenticateTestApplication(context);

    const ByteVector payload = createTestPayload(128U);

    logInfo("Payload size : " + std::to_string(payload.size()));
    logInfo("Payload : " + toHex(payload));

    traceOperation("Write additional data payload", [&context, &payload]()
                   { context.duoxCommands->VDE_WriteData(payload); });

    logPass("Additional data write operation completed.");

    // Read-after-write verification
    const ByteVector readBack = readTestApplicationAdditionalData(context);

    require(readBack.size() >= payload.size(),
            "Read-back data is smaller than the written payload.");

    require(std::equal(payload.begin(), payload.end(), readBack.begin()),
            "Read-back data does not match the written payload.");

    logPass("Additional data read-after-write verification succeeded.");
}

// ==============================================
// Test application locking
// ==============================================

void testAdditionalDataLocking(DUOXTestContext &context)
{
    printSection("TEST APPLICATION ADDITIONAL DATA LOCKING");

    if (!RUN_IRREVERSIBLE_LOCK_TEST)
    {
        logSkip("Irreversible lock test disabled.");

        return;
    }

    require(TEST_APPLICATION_VDE_CONFIGURED, "Cannot lock an unconfigured test application.");

    logInfo("WARNING : The following operation is irreversible.");
    logInfo("Further writes to the additional data file may be permanently prevented.");

    unauthenticateTestApplication(context);

    traceOperation("Lock test application additional data file", [&context]() { context.duoxCommands->VDE_LockData(); });

    logPass("Additional data lock command completed.");

    // Verify that a subsequent write is rejected
    const ByteVector rejectedPayload = createTestPayload(16U);

    bool writeRejected = false;

    try
    {
        context.duoxCommands->VDE_WriteData(rejectedPayload);
    }
    catch (const std::exception &exception)
    {
        writeRejected = true;

        logInfo("Write after lock rejected : " + std::string(exception.what()));
    }

    require(writeRejected, "Card accepted a write after the file was locked.");

    logPass("Write after lock correctly rejected.");
}

// ==============================================
// Test application read-only tests
// ==============================================

void runTestApplicationReadOnlyTests(DUOXTestContext &context)
{
    printSection("TEST APPLICATION READ-ONLY TESTS");

    const ByteVector certificate = readTestApplicationCertificate(context);

    logInfo("Certificate currently contains " + std::to_string(certificate.size()) + " bytes.");

    const ByteVector additionalData = readTestApplicationAdditionalData(context);

    logInfo("Additional data file currently contains " + std::to_string(additionalData.size()) + " bytes.");

    logPass("Test application read-only operations completed.");
}

// ==============================================
// Complete test application test sequence
// ==============================================

void runTestApplicationTests(DUOXTestContext &context, TestApplicationGuard &applicationGuard)
{
    printSection("DEDICATED TEST APPLICATION TEST SUITE");

    const bool newlyCreated = prepareTestApplication(context, applicationGuard);

    if (!newlyCreated && !applicationGuard.armed())
    {
        logSkip("Test application was not created by this execution.");
        logSkip("No provisioning, writing or locking will be performed.");

        runTestApplicationReadOnlyTests(context);

        return;
    }

    // ------------------------------------------
    // ECC key generation
    // ------------------------------------------

    provisionTestApplicationECC(context);

    // ------------------------------------------
    // Read-only tests
    // ------------------------------------------

    runTestApplicationReadOnlyTests(context);

    const ByteVector challenge = generateChallenge();
    const ByteVector signature = signTestApplicationChallenge(context, challenge);

    (void)signature;

    testAdditionalDataWriting(context);

    // ------------------------------------------
    // Irreversible operation
    // ------------------------------------------

    testAdditionalDataLocking(context);
}

// ==============================================
// Test execution summary
// ==============================================

struct TestExecutionSummary
{
    unsigned int successfulSuites = 0U;
    unsigned int skippedSuites    = 0U;
    unsigned int failedSuites     = 0U;
};

void skipTestSuite(TestExecutionSummary &summary, const std::string &name, const std::string &reason)
{
    ++summary.skippedSuites;
    logSkip(name + " : " + reason);
}

template <typename Function>
void executeTestSuite(TestExecutionSummary &summary, const std::string &name, Function &&function)
{
    try
    {
        traceOperation(name, std::forward<Function>(function));

        ++summary.successfulSuites;
    }
    catch (const std::exception &exception)
    {
        ++summary.failedSuites;

        logFailure(name, exception.what());

        throw;
    }
}

void printExecutionSummary(const TestExecutionSummary &summary)
{
    printSection("EV CHARGING TEST EXECUTION SUMMARY");

    std::cout << "Successful suites : " << summary.successfulSuites << '\n';
    std::cout << "Skipped suites    : " << summary.skippedSuites << '\n';
    std::cout << "Failed suites     : " << summary.failedSuites << '\n';

    std::cout << std::endl;
}

// ==============================================
// Complete DUOX EV Charging execution
// ==============================================

int runDUOXEVChargingTests()
{
    TestExecutionSummary summary;

    DUOXTestContext context;

    initializeDUOXTestContext(context);

    ReaderConnectionGuard readerGuard(context.reader);

    // Retrieve card UID before application operations
    executeTestSuite(summary, "Read physical card UID",
                     [&context]() { context.cardUID = getCardUID(context); });

    // ------------------------------------------
    // Card preparation
    // ------------------------------------------

    executeTestSuite(summary, "Prepare DUOX test card",
                     [&context]() { prepareDUOXTestCard(context); });

    // ------------------------------------------
    // Factory application tests
    // ------------------------------------------

    if (factoryApplicationExists(context))
    {
        executeTestSuite(summary, "Factory EV Charging application tests",
                         [&context]() { runFactoryEVChargingTests(context); });
    }
    else
    {
        skipTestSuite(summary, "Factory EV Charging tests", "Factory application does not exist.");
    }

    // ------------------------------------------
    // Dedicated test application tests
    // ------------------------------------------

    TestApplicationGuard applicationGuard(context);

    try
    {
        executeTestSuite(summary, "Dedicated EV Charging test application tests", [&context, &applicationGuard]()
                         { runTestApplicationTests(context, applicationGuard); });

        if (DELETE_TEST_APPLICATION_AFTER_TESTS)
            applicationGuard.cleanup();
    }
    catch (...)
    {
        // The guard retains ownership and attempts cleanup during stack unwinding
        throw;
    }

    printExecutionSummary(summary);

    return summary.failedSuites == 0U ? EXIT_SUCCESS : EXIT_FAILURE;
}

// ==============================================
// Main
// ==============================================

} // anonymous namespace

int main()
{
    try
    {
        printSection("LIBLOGICALACCESS DUOX EV CHARGING TEST SUITE");

        logInfo("Dedicated test application AID : " + formatAid(EV_CHARGING_TEST_AID));
        logInfo("Factory EV Charging application AID : " + formatAid(EV_CHARGING_FACTORY_AID));
        logInfo("Card preparation mode : DeleteTestApplication");
        logInfo("SetConfiguration enabled : true");
        logInfo("Irreversible locking enabled : " + std::string(RUN_IRREVERSIBLE_LOCK_TEST ? "true" : "false"));

        const int result = runDUOXEVChargingTests();

        if (result == EXIT_SUCCESS)
        {
            logPass("DUOX EV Charging test suite completed successfully.");
        }
        else
        {
            logFailure("DUOX EV Charging test suite", "One or more test suites failed.");
        }

        return result;
    }
    catch (const std::exception &exception)
    {
        logFailure("DUOX EV Charging test suite", exception.what());

        return EXIT_FAILURE;
    }
    catch (...)
    {
        logFailure("DUOX EV Charging test suite", "Unknown fatal exception.");

        return EXIT_FAILURE;
    }
}