#include <algorithm>
#include <ctime>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/desfire/desfireev1commands.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>

#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816commands.hpp>

namespace
{

enum class SAMAuthenticationMode
{
    AuthenticateHost,
    LockUnlock
};

// DESFire command
struct DESFireContext
{
    std::shared_ptr<logicalaccess::DESFireCommands> commands;
    std::shared_ptr<logicalaccess::DESFireEV1Commands> ev1;
};

// Configuration
constexpr char READER_PROVIDER[] = "PCSC";
constexpr char SAM_READER_NAME[] = "HID Global OMNIKEY 5422 Smartcard Reader 0";
constexpr char SAM_TYPE[] = "SAM_AUTO";

constexpr std::size_t CONTACTLESS_READER_INDEX = 1;

constexpr std::size_t TOTAL_TEST_STEPS = 7;

constexpr std::uint32_t TEST_APPLICATION_ID = 1U;
constexpr std::uint8_t PICC_MASTER_KEY      = 0;
constexpr std::uint8_t SAM_KEY_SLOT         = 2;

constexpr unsigned int DESFIRE_TIMEOUT_MS = 15000;

constexpr unsigned int DESFIRE_TIMEOUT_SECONDS = DESFIRE_TIMEOUT_MS / 1000;

constexpr SAMAuthenticationMode SAM_AUTHENTICATION_MODE = SAMAuthenticationMode::LockUnlock;
//constexpr SAMAuthenticationMode SAM_AUTHENTICATION_MODE = SAMAuthenticationMode::AuthenticateHost;

constexpr const char *SAM_AUTHENTICATION_MODE_NAME =
    SAM_AUTHENTICATION_MODE == SAMAuthenticationMode::AuthenticateHost ? "AuthenticateHost" : "LockUnlock";

// Logging
void printSeparator()
{
    std::cout << "------------------------------------------------------------\n";
}

void printBanner()
{
    printSeparator();
    std::cout << "DESFire EV1 + SAM Integration Test\n";
    printSeparator();
    std::cout << "\n"
              << "This test validates the complete authentication flow between\n"
              << "a DESFire EV1 card and a Secure Access Module (SAM)\n"
              << "\n"
              << "Hardware setup\n"
              << "==============\n"
              << "  PC/SC reader #0\n"
              << "      HID Global OMNIKEY 5422 Smartcard Reader 0\n"
              << "      -> Must contain a configured SAM\n"
              << "\n"
              << "  PC/SC reader #1\n"
              << "      HID Global OMNIKEY 5422CL Smartcard Reader 0\n"
              << "      -> Must contain a DESFire EV1 card\n"
              << "\n"
              << "Hardware initialization\n"
              << "=======================\n"
              << "  - Initialize PC/SC reader\n"
              << "  - Initialize and validate SAM\n"
              << "  - Wait for DESFire EV1 card\n"
              << "\n"
              << "DESFire test sequence\n"
              << "=======================\n"
              << "  1 - Prepare card (erase content / remove test application)\n"
              << "  2 - Create an AES application\n"
              << "  3 - Select the application\n"
              << "  4 - Authenticate through the SAM\n"
              << "  5 - Create an encrypted data file\n"
              << "  6 - Write encrypted data\n"
              << "  7 - Change the application key\n"
              << "\n"
              << "Requirements\n"
              << "============\n"
              << "  - A configured SAM must already be inserted in reader #0\n"
              << "  - A DESFire EV1 card must be presented on reader #1\n"
              << "\n"
              << "SAM configuration\n"
              << "=================\n"
              << "  Reader : " << SAM_READER_NAME << '\n'
              << "  Mode   : " << SAM_AUTHENTICATION_MODE_NAME << '\n'
              << '\n';
    printSeparator();
    std::cout << std::endl;
}

void logStep(unsigned int current, unsigned int total, const std::string &message)
{
    std::cout << std::endl;
    std::cout << "[" << current << "/" << total << "] " << message << "..." << std::endl;
}

void logInfo(const std::string &message)
{
    std::cout << "[INFO] " << message << std::endl;
}

void logSuccess(const std::string &message)
{
    std::cout << "[ OK ] " << message << std::endl;
}

// DESFire
std::shared_ptr<logicalaccess::DESFireKey> createDefaultAESKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    key->setData(logicalaccess::BufferHelper::fromHexString("00000000000000000000000000000000"));
    return key;
}

// DESFire
std::shared_ptr<logicalaccess::DESFireKey> createDefaultDESKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_DES);
    key->setData(logicalaccess::BufferHelper::fromHexString("00000000000000000000000000000000"));
    return key;
}

std::shared_ptr<logicalaccess::DESFireKey> createSAMAESKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    auto storage = std::make_shared<logicalaccess::SAMKeyStorage>();
    storage->setKeySlot(SAM_KEY_SLOT);
    key->setKeyVersion(0);
    key->setKeyStorage(storage);
    return key;
}

std::shared_ptr<logicalaccess::DESFireKey> createSAMUnlockKey()
{
    auto key = std::make_shared<logicalaccess::DESFireKey>();
    key->setKeyType(logicalaccess::DF_KEY_AES);
    key->setData(logicalaccess::BufferHelper::fromHexString("00000000000000000000000000000000"));
    return key;
}

void resetCard(const std::shared_ptr<logicalaccess::DESFireCommands> &commands)
{
    commands->selectApplication(0);
    commands->authenticate(PICC_MASTER_KEY, createDefaultDESKey());
    commands->erase();
}

// Reader configuration
std::shared_ptr<logicalaccess::ReaderConfiguration> createReaderConfiguration()
{
    auto configuration = std::make_shared<logicalaccess::ReaderConfiguration>();
    auto provider = logicalaccess::LibraryManager::getInstance()->getReaderProvider(READER_PROVIDER);

    if (!provider)
        throw std::runtime_error("Unable to load the PC/SC reader provider.");

    configuration->setReaderProvider(provider);
    const auto readers = provider->getReaderList();

    if (readers.empty())
        throw std::runtime_error("No PC/SC reader is available on this system.");
    if (CONTACTLESS_READER_INDEX >= readers.size())
        throw std::runtime_error("The expected contactless reader (index 1) is not available.");

    configuration->setReaderUnit(readers.at(CONTACTLESS_READER_INDEX));
    logInfo("Reader provider : " + provider->getRPType());
    logInfo("Contactless reader : " + readers.at(CONTACTLESS_READER_INDEX)->getName());

    return configuration;
}

enum class CardPreparationMode
{
    EraseCard,
    DeleteTestApplication
};

constexpr CardPreparationMode PREPARATION_MODE = CardPreparationMode::EraseCard;

class ReaderConnectionGuard
{
  public:
    explicit ReaderConnectionGuard(const std::shared_ptr<logicalaccess::ReaderUnit> &reader) : reader_(reader)
    {
        if (!reader_)
            throw std::runtime_error("Reader connection initialization failed : the selected reader instance is null.");
        reader_->connectToReader();
    }

    ~ReaderConnectionGuard()
    {
        try
        {
            if (reader_)
                reader_->disconnect();
        }
        catch (const std::exception &exception)
        {
            std::cerr << "Reader disconnect failed during cleanup : " << exception.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Reader disconnect failed during cleanup with unknown error." << std::endl;
        }
    }

    ReaderConnectionGuard(const ReaderConnectionGuard &)            = delete;
    ReaderConnectionGuard &operator=(const ReaderConnectionGuard &) = delete;

  private:
    std::shared_ptr<logicalaccess::ReaderUnit> reader_;
};

// SAM configuration
void configureSAM(const std::shared_ptr<logicalaccess::ReaderConfiguration> &configuration)
{
    auto isoReader = std::dynamic_pointer_cast<logicalaccess::ISO7816ReaderUnit>(configuration->getReaderUnit());

    if (!isoReader)
        throw std::runtime_error("The selected reader does not implement ISO7816ReaderUnit.");

    auto isoConfiguration =
        std::dynamic_pointer_cast<logicalaccess::ISO7816ReaderUnitConfiguration>(isoReader->getConfiguration());

    if (!isoConfiguration)
        throw std::runtime_error("Unable to retrieve the ISO7816 reader configuration.");

    isoConfiguration->setSAMReaderName(SAM_READER_NAME);
    isoConfiguration->setSAMType(SAM_TYPE);
    isoConfiguration->setSAMUnlockKey(createSAMUnlockKey(), PICC_MASTER_KEY);

    // The SAM performs the authentication while remaining unlocked.
    isoConfiguration->setUseSAMAuthenticateHost(SAM_AUTHENTICATION_MODE == SAMAuthenticationMode::AuthenticateHost);

    // Automatically connect to the SAM reader.
    isoConfiguration->setAutoConnectToSAMReader(true);

    logInfo("SAM reader : " + std::string(SAM_READER_NAME));
    logInfo("SAM mode : " + std::string(SAM_AUTHENTICATION_MODE_NAME));
    logSuccess("SAM configuration completed.");
}

// Card detection
std::shared_ptr<logicalaccess::Chip> waitForCard(const std::shared_ptr<logicalaccess::ReaderConfiguration> &configuration)
{
    auto reader = configuration->getReaderUnit();
    reader->setCardType("DESFireEV1");

    logInfo("Waiting for DESFire EV1 card insertion (timeout : " + std::to_string(DESFIRE_TIMEOUT_SECONDS) + " seconds)...");

    if (!reader->waitInsertion(DESFIRE_TIMEOUT_MS))
        throw std::runtime_error("No DESFire EV1 card was detected within the timeout.");
    if (!reader->connect())
        throw std::runtime_error("Failed to establish communication with the card.");

    auto chip = reader->getSingleChip();

    if (!chip)
        throw std::runtime_error("The reader did not return a valid chip instance.");

    logSuccess("Connected to \"" + reader->getConnectedName() + "\"");
    logInfo("Card type : " + chip->getCardType());
    const auto uid = reader->getNumber(chip);
    logInfo("Card UID  : " + logicalaccess::BufferHelper::getHex(uid));
    return chip;
}

DESFireContext createDESFireContext(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    DESFireContext context;
    context.commands = std::dynamic_pointer_cast<logicalaccess::DESFireCommands>(chip->getCommands());
    context.ev1 = std::dynamic_pointer_cast<logicalaccess::DESFireEV1Commands>(chip->getCommands());

    if (!context.commands)
        throw std::runtime_error("The detected card does not expose DESFireCommands.");
    if (!context.ev1)
        throw std::runtime_error("The detected card does not expose DESFireEV1Commands. Please present a DESFire EV1 card.");

    return context;
}

// Card preparation
void deleteExistingApplication(const DESFireContext &context)
{
    logStep(1, TOTAL_TEST_STEPS, "Removing previous test application if present");
    context.commands->selectApplication(0);
    const auto applications = context.commands->getApplicationIDs();

    if (std::find(applications.begin(), applications.end(), TEST_APPLICATION_ID) == applications.end())
    {
        logInfo("No previous test application found.");
        return;
    }

    logInfo("A previous test application was found.");
    context.commands->authenticate(PICC_MASTER_KEY, createDefaultDESKey());
    context.commands->deleteApplication(TEST_APPLICATION_ID);
    logSuccess("Existing test application removed.");
}

void prepareCard(const DESFireContext &context)
{
    logStep(1, TOTAL_TEST_STEPS, "Preparing card.");
    if (PREPARATION_MODE == CardPreparationMode::EraseCard)
    {
        logInfo("Card preparation mode : full erase.");
        resetCard(context.commands);
    }
    else
    {
        logInfo("Card preparation mode : removing test application only.");
        deleteExistingApplication(context);
    }
    logSuccess("Card preparation completed.");
}

// DESFire integration test steps
void createApplication(const DESFireContext &context)
{
    logStep(2, TOTAL_TEST_STEPS, "Creating AES application");
    context.ev1->createApplication(TEST_APPLICATION_ID, logicalaccess::KS_DEFAULT, 2, logicalaccess::DF_KEY_AES);
    logStep(3, TOTAL_TEST_STEPS, "Selecting application");
    context.commands->selectApplication(TEST_APPLICATION_ID);
}

std::shared_ptr<logicalaccess::DESFireKey> authenticate(const DESFireContext &context)
{
    logStep(4, TOTAL_TEST_STEPS, "Authenticating through the SAM");
    //auto key = createDefaultAESKey();
    auto key = createSAMAESKey();
    context.commands->authenticate(PICC_MASTER_KEY, key);
    logSuccess("Authentication successful.");
    return key;
}

void createEncryptedFile(const DESFireContext &context)
{
    logStep(5, TOTAL_TEST_STEPS, "Creating encrypted standard data file");
    logicalaccess::DESFireAccessRights rights;
    rights.readAccess         = logicalaccess::AR_KEY0;
    rights.writeAccess        = logicalaccess::AR_KEY0;
    rights.readAndWriteAccess = logicalaccess::AR_KEY0;
    rights.changeAccess       = logicalaccess::AR_KEY0;
    context.commands->createStdDataFile(0, logicalaccess::CM_ENCRYPT, rights, 8);
    logSuccess("Encrypted file created.");
}

void writeEncryptedData(const DESFireContext &context)
{
    logStep(6, TOTAL_TEST_STEPS, "Writing encrypted data");
    const ByteVector payload = {0x01, 0x02, 0x03};
    context.commands->writeData(0, 0, payload, logicalaccess::CM_ENCRYPT);
    logSuccess("Encrypted write completed.");
}

void changeApplicationKey(const DESFireContext &context, const std::shared_ptr<logicalaccess::DESFireKey> &key)
{
    logStep(7, TOTAL_TEST_STEPS, "Changing application key");
    context.commands->changeKey(PICC_MASTER_KEY, key);
    logSuccess("Application key updated.");
}

// Complete integration test
void executeIntegrationTest(const std::shared_ptr<logicalaccess::Chip> &chip)
{
    auto context = createDESFireContext(chip);
    prepareCard(context);
    createApplication(context);

    const auto key = authenticate(context);
    createEncryptedFile(context);
    writeEncryptedData(context);
    changeApplicationKey(context, key);

    std::cout << std::endl;
    printSeparator();
    logSuccess("DESFire EV1 + SAM integration test completed successfully.");
    printSeparator();
}

bool askRetry(const std::string &message)
{
    std::cout << '\n' << message << " (Y/N) : ";
    char answer{};
    std::cin >> answer;
    return answer == 'y' || answer == 'Y';
}
}

int main()
{
    bool firstRun = true;

    printBanner();

    while (true)
    {
        try
        {
            if (!firstRun)
                printSeparator();
            const auto now = std::time(nullptr);
            logInfo(std::string("Time start : ") + std::ctime(&now));
            auto readerConfiguration = createReaderConfiguration();
            configureSAM(readerConfiguration);

            ReaderConnectionGuard connection(readerConfiguration->getReaderUnit());
            auto chip = waitForCard(readerConfiguration);
            executeIntegrationTest(chip);

            if (!askRetry("Run another test ?"))
                return EXIT_SUCCESS;
        }
        catch (const std::exception &exception)
        {
            printSeparator();
            std::cerr << "[FAIL] Integration test failed\n\n" << "Reason :\n" << exception.what() << '\n';
            printSeparator();
            if (!askRetry("Retry ?"))
                return EXIT_FAILURE;
        }
        catch (...)
        {
            printSeparator();
            std::cerr << "[FAIL] Integration test failed due to an unknown error.\n";
            printSeparator();
            if (!askRetry("Retry ?"))
                return EXIT_FAILURE;
        }
        firstRun = false;
    }
    return EXIT_SUCCESS; // Unreachable but avoids compiler warning
}