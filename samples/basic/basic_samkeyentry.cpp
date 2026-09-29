/**
 * \file basic_samkeyentry.cpp
 * \brief Tests for SAMKeyEntry serialization, deserialization, SET handling,
 * and read only validation against a physical SAM.
 *
 * This test intentionally remains an integration or functional test rather than a unit-test abstraction.
 * The goal is to preserve exhaustive hardware level validation while making
 * ownership, lifecycle, invariants and diagnostics explicit
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <type_traits>

#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/readerproviders/readerconfiguration.hpp>

#include <logicalaccess/plugins/cards/samav/samkeyentry.hpp>
#include <logicalaccess/plugins/cards/samav/sambasickeyentry.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/samav3iso7816commands.hpp>

#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>

namespace
{

// =================================================
// Configuration
// =================================================

/*
 * IMPORTANT :
 * This test suite intentionally contains NO SAM key writing command.
 * The only SAM command used against a physical SAM is SAM_GetKeyEntry()
 *
 * Host authentication is also performed, because real KST read requires an authenticated SAM session.
 *
 * There is no : changeKey, SET update, ExtSET update, key import/export, key deletion/creation
 */

constexpr bool REAUTH_AFTER_EACH_TEST = false;

/*
 * Read only slots to inspect on the real SAM.
 *
 * More slots can be added but SAM_GetKeyEntry may expose sensitive information to the host.
 * The test never prints key material.
 */
const std::vector<std::uint8_t> LIVE_KEY_SLOTS = {0x00};

// =================================================
// Generic test infrastructure
// =================================================

struct TestHooks
{
    std::function<void()> beforeEach;
    std::function<void()> afterEach;
};

template <typename TestCase, typename TestFunction, typename ResultFunction>
void runTestSuite(const std::string &name, const std::vector<TestCase> &tests,
                  TestFunction testFunction, ResultFunction resultFunction,
                  const TestHooks &hooks = {})
{
    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << " TEST SUITE : " << name << '\n';
    std::cout << "=================================================\n";

    std::size_t passed = 0;
    std::size_t failed = 0;

    for (std::size_t i = 0; i < tests.size(); ++i)
    {
        const auto &test = tests[i];

        std::cout << "\n";
        std::cout << "-------------------------------------------------\n";
        std::cout << " Test " << (i + 1) << "/" << tests.size() << '\n';
        std::cout << "-------------------------------------------------\n";

        if (hooks.beforeEach)
            hooks.beforeEach();

        bool success = false;

        try
        {
            testFunction(test);
            success = true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[EXCEPTION] " << e.what() << '\n';
        }
        catch (...)
        {
            std::cerr << "[EXCEPTION] Unknown exception.\n";
        }

        try
        {
            resultFunction(test, success);
        }
        catch (const std::exception &e)
        {
            ++failed;

            std::cerr << "[FAIL] " << e.what() << '\n';

            if (hooks.afterEach)
                hooks.afterEach();

            continue;
        }

        if (success)
            ++passed;
        else
            ++failed;

        if (hooks.afterEach)
            hooks.afterEach();
    }

    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << " RESULT : " << name << '\n';
    std::cout << " PASSED : " << passed << '\n';
    std::cout << " FAILED : " << failed << '\n';
    std::cout << "=================================================\n";

    if (failed != 0)
        throw std::runtime_error("Test suite failed : " + name);
}

template <typename Function>
void runSingleTest(const std::string &name, Function &&function)
{
    std::cout << "\n";
    std::cout << "[TEST] " << name << '\n';

    try
    {
        function();
        std::cout << "[PASS] " << name << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "[FAIL] " << name << " : " << e.what() << '\n';
        throw;
    }
}

// =================================================
// Assertions
// =================================================

void require(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}

template <typename T, typename U>
void requireEqual(const T &actual, const U &expected, const std::string &message)
{
    if (!(actual == expected))
        throw std::runtime_error(message);
}

void requireThrows(const std::string &name, const std::function<void()> &function)
{
    bool thrown = false;

    try
    {
        function();
    }
    catch (...)
    {
        thrown = true;
    }

    if (!thrown)
        throw std::runtime_error("Expected exception was not thrown : " + name);
}

// =================================================
// Hex helpers
// =================================================

std::string toHex(const unsigned char *data, std::size_t size)
{
    std::ostringstream stream;
    stream << std::hex << std::uppercase << std::setfill('0');

    for (std::size_t i = 0; i < size; ++i)
    {
        if (i != 0)
            stream << ' ';
        stream << std::setw(2) << static_cast<unsigned int>(data[i]);
    }

    return stream.str();
}

std::string toHex(const ByteVector &data)
{
    if (data.empty())
        return {};

    return toHex(data.data(), data.size());
}

// =================================================
// Byte comparison helpers
// =================================================

template <typename T>
bool rawObjectEquals(const T &lhs, const T &rhs)
{
    static_assert(std::is_trivially_copyable<T>::value, "rawObjectEquals requires trivially copyable types.");
    return std::memcmp(&lhs, &rhs, sizeof(T)) == 0;
}

template <typename T>
void requireRawObjectEqual(const T &actual, const T &expected, const std::string &message)
{
    if (!rawObjectEquals(actual, expected))
        throw std::runtime_error(message);
}

// =================================================
// SET structure inspection
// =================================================

template <typename T>
void requireSETStructureIsValid(const std::string &name)
{
    requireEqual(sizeof(T), static_cast<std::size_t>(16), name + " must contain exactly 16 bytes.");
}

// =================================================
// SET encode/decode tests
// =================================================

template <typename SET>
void testSETRoundTrip(const std::string &name)
{
    requireSETStructureIsValid<SET>(name);

    // Each of the 16 structure bytes contains either 0 or 1
    SET original{};

    unsigned char *raw = reinterpret_cast<unsigned char *>(&original);

    for (std::size_t i = 0; i < sizeof(SET); ++i)
        raw[i] = static_cast<unsigned char>((i % 3U) == 0U);

    unsigned char encoded[2] = {0, 0};
    logicalaccess::samav_detail::encodeSET(original, encoded);

    const SET decoded = logicalaccess::samav_detail::decodeSET<SET>(encoded);
    requireRawObjectEqual(decoded, original, name + " encode/decode round-trip failed.");
}

template <typename SET>
void testSETAllBits(const std::string &name)
{
    requireSETStructureIsValid<SET>(name);

    /*
     * Test each individual bit independently to catch :
     * - reversed bit order
     * - reversed byte order
     * - off-by-one errors
     * - wrong shift direction
     */
    for (std::size_t byteIndex = 0; byteIndex < 2; ++byteIndex)
    {
        for (std::size_t bitIndex = 0; bitIndex < 8; ++bitIndex)
        {
            SET original{};

            unsigned char *raw = reinterpret_cast<unsigned char *>(&original);
            raw[byteIndex * 8U + bitIndex] = 1;

            unsigned char encoded[2] = {0, 0};
            logicalaccess::samav_detail::encodeSET(original, encoded);

            const unsigned char expected = static_cast<unsigned char>(1U << bitIndex);
            requireEqual(encoded[byteIndex], expected, name + " encoded wrong bit position");

            const SET decoded = logicalaccess::samav_detail::decodeSET<SET>(encoded);
            requireRawObjectEqual(decoded, original, name + " individual-bit round-trip failed");
        }
    }
}

// =================================================
// Synthetic KST information
// =================================================

template <typename T>
T makeSyntheticInformation()
{
    T information{};
    unsigned char *raw = reinterpret_cast<unsigned char *>(&information);

    for (std::size_t i = 0; i < sizeof(T); ++i)
        raw[i] = static_cast<unsigned char>((0x11U + (i * 0x17U)) & 0xFFU);

    return information;
}

/*
 * ExtSET fields have version specific sizes.
 *
 * This function gives AV2/AV3 deterministic synthetic values so the extra serialization bytes are exercised
 */
template <>
logicalaccess::KeyEntryAV2Information makeSyntheticInformation<logicalaccess::KeyEntryAV2Information>()
{
    logicalaccess::KeyEntryAV2Information information{};

    information.desfireAid[0] = 0x12;
    information.desfireAid[1] = 0x34;
    information.desfireAid[2] = 0x56;

    information.desfirekeyno = 0x21;
    information.cekno        = 0x32;
    information.cekv         = 0x43;
    information.kuc          = 0x54;

    information.set[0] = 0x18;
    information.set[1] = 0xA5;

    information.vera = 0x65;
    information.verb = 0x76;
    information.verc = 0x87;

    information.ExtSET = 0x98;

    return information;
}

template <>
logicalaccess::KeyEntryAV3Information makeSyntheticInformation<logicalaccess::KeyEntryAV3Information>()
{
    logicalaccess::KeyEntryAV3Information information{};

    information.desfireAid[0] = 0x12;
    information.desfireAid[1] = 0x34;
    information.desfireAid[2] = 0x56;

    information.desfirekeyno = 0x21;
    information.cekno        = 0x32;
    information.cekv         = 0x43;
    information.kuc          = 0x54;

    information.set[0] = 0x38;
    information.set[1] = 0xA5;

    information.vera = 0x65;
    information.verb = 0x76;
    information.verc = 0x87;

    information.ExtSET    = 0x1234;
    information.keyNoAEK  = 0x56;
    information.keyVerAEK = 0x78;

    return information;
}

// =================================================
// Synthetic SET values
// =================================================

template <typename SET>
SET makeSyntheticSET()
{
    SET set{};
    unsigned char *raw = reinterpret_cast<unsigned char *>(&set);

    for (std::size_t i = 0; i < sizeof(SET); ++i)
        raw[i] = static_cast<unsigned char>((i % 2U) != 0U);

    return set;
}

// =================================================
// KST object construction helpers
// =================================================

template <typename Entry>
void initialiseSyntheticKeyMaterial(Entry &entry, logicalaccess::SAMKeyType keyType)
{
    const std::size_t keyLength = [&]()
    {
        switch (keyType)
        {
        case logicalaccess::SAMKeyType::SAM_KEY_MIFARE:
        case logicalaccess::SAMKeyType::SAM_KEY_DES:
        case logicalaccess::SAMKeyType::SAM_KEY_AES128:
            return static_cast<std::size_t>(SAM_KEY_SIZE_128);

        case logicalaccess::SAMKeyType::SAM_KEY_3K3DES:
        case logicalaccess::SAMKeyType::SAM_KEY_AES192:
            return static_cast<std::size_t>(SAM_KEY_SIZE_192);

        case logicalaccess::SAMKeyType::SAM_KEY_AES256:
            return static_cast<std::size_t>(SAM_KEY_SIZE_256);

        default: throw std::invalid_argument("Unsupported synthetic SAM key type.");
        }
    }();

    std::vector<ByteVector> keys;

    const unsigned char keyCount = [&]()
    {
        switch (keyType)
        {
        case logicalaccess::SAMKeyType::SAM_KEY_MIFARE:
        case logicalaccess::SAMKeyType::SAM_KEY_DES:
        case logicalaccess::SAMKeyType::SAM_KEY_AES128:
            return static_cast<unsigned char>(3);

        case logicalaccess::SAMKeyType::SAM_KEY_3K3DES:
        case logicalaccess::SAMKeyType::SAM_KEY_AES192:
            return static_cast<unsigned char>(2);

        case logicalaccess::SAMKeyType::SAM_KEY_AES256:
            return static_cast<unsigned char>(1);

        default: throw std::invalid_argument("Unsupported synthetic SAM key type.");
        }
    }();

    for (unsigned char keyIndex = 0; keyIndex < keyCount; ++keyIndex)
    {
        ByteVector key(keyLength);

        for (std::size_t i = 0; i < keyLength; ++i)
            key[i] = static_cast<unsigned char>((0x10U + keyIndex * 0x20U + i) & 0xFFU);

        keys.push_back(key);
    }

    entry.setKeysData(keys, keyType);
}

// =================================================
// Information comparison
// =================================================

template <typename T>
void requireKeyInformationEqual(const T &actual, const T &expected)
{
    requireRawObjectEqual(actual, expected, "KST key entry information changed during round-trip.");
}

// =================================================
// Offline SAMKeyEntry serialization tests
// =================================================

template <typename Entry, typename SET>
void testSAMKeyEntryRoundTrip(const std::string &name, logicalaccess::SAMKeyType keyType)
{
    std::cout << " [ROUND-TRIP] " << name << '\n';

    Entry original;
    original.setKeyType(keyType);

    initialiseSyntheticKeyMaterial(original, keyType);

    const auto information = makeSyntheticInformation<typename Entry::KeyEntryInformationType>();

    // If current SAMKeyEntry implementation doesn't have a type alias named KeyEntryInformationType
    original.setKeyEntryInformation(information);

    const SET syntheticSET = makeSyntheticSET<SET>();
    original.setSET(syntheticSET);

    original.setSETKeyTypeFromKeyType();

    // Re-read the information after setKeysData() writes the key type into SET
    const auto expectedInformation = original.getKeyEntryInformation();

    std::cout << "  supplied key type : 0x" << std::hex << std::uppercase
              << static_cast<unsigned int>(keyType) << '\n';
    std::cout << "  entry key type    : 0x" << std::hex << std::uppercase
              << static_cast<unsigned int>(original.getKeyType()) << std::dec << '\n';

    requireEqual(original.getKeyType(), keyType, name + " synthetic entry key type mismatch.");

    const ByteVector serialized = original.serializeKSTKeyEntry();

    std::cout << "  serialized size    : " << serialized.size() << '\n';
    std::cout << "  serialized bytes  : ";
    for (std::size_t i = 0; i < serialized.size(); ++i)
    {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned int>(serialized[i]) << ' ';
    }
    std::cout << std::dec << std::setfill(' ') << '\n';

    require(!serialized.empty(), name + " produced empty KST serialization.");

    std::cout << "  original.getKeyType() : 0x" << std::hex << std::uppercase
              << static_cast<unsigned int>(original.getKeyType()) << '\n';
    std::cout << "  supplied keyType      : 0x" << static_cast<unsigned int>(keyType) << std::dec << '\n';

    // Deserialize into completely independent object
    Entry restored;
    restored.deserializeKSTKeyEntry(serialized.data(), serialized.size(), keyType);
    requireEqual(restored.getKeyType(), original.getKeyType(), name + " key type changed after deserialization.");
    requireEqual(restored.getKeyNb(), original.getKeyNb(), name + " key count changed after deserialization.");
    requireEqual(restored.getLength(), original.getLength(), name + " key length changed after deserialization.");

    auto expectedRoundTripInformation = expectedInformation;
    if (original.getKeyNb() < 3)
        expectedRoundTripInformation.verc = 0;
    if (original.getKeyNb() < 2)
        expectedRoundTripInformation.verb = 0;

    requireKeyInformationEqual(restored.getKeyEntryInformation(), expectedRoundTripInformation);

    // SET structure round-trip
    const SET restoredSET = restored.getSETStruct();
    const SET originalSET = original.getSETStruct();

    requireRawObjectEqual(restoredSET, originalSET, name + " SET structure changed after KST round-trip.");

    // Serialize again to check if serializer is deterministic : it must produce byte-for-byte identical KST 
    const ByteVector reserialized = restored.serializeKSTKeyEntry();

    requireEqual(reserialized, serialized, name + " serialize/deserialize/serialize is not byte-stable");
    std::cout << "  size      : " << serialized.size() << " bytes\n"
              << "  key count : " << static_cast<unsigned int>(restored.getKeyNb())
              << '\n'
              << "  key size  : " << restored.getLength() << " bytes\n"
              << "  result    : OK\n";
}

// =================================================
// Metadata-only round-trip
// =================================================

void checkMetadataExtra(const logicalaccess::KeyEntryAV1Information &,
                        const logicalaccess::KeyEntryAV1Information &,
                        const std::string &)
{
    // SAM AV1 has no ExtSET
}

void checkMetadataExtra(const logicalaccess::KeyEntryAV2Information &restoredInfo,
                        const logicalaccess::KeyEntryAV2Information &originalInfo,
                        const std::string &name)
{
    requireEqual(restoredInfo.ExtSET, originalInfo.ExtSET, name + " ExtSET changed during metadata round-trip");
}

void checkMetadataExtra(const logicalaccess::KeyEntryAV3Information &restoredInfo,
                        const logicalaccess::KeyEntryAV3Information &originalInfo,
                        const std::string &name)
{
    requireEqual(restoredInfo.ExtSET, originalInfo.ExtSET,
                 name + " ExtSET changed during metadata round-trip.");

    requireEqual(restoredInfo.keyNoAEK, originalInfo.keyNoAEK,
                 name + " keyNoAEK changed during metadata round-trip.");

    requireEqual(restoredInfo.keyVerAEK, originalInfo.keyVerAEK,
                 name + " keyVerAEK changed during metadata round-trip.");
}

template <typename Entry>
void testMetadataRoundTrip(const std::string &name, logicalaccess::SAMKeyType keyType)
{
    Entry original;

    initialiseSyntheticKeyMaterial(original, keyType);

    const auto information = makeSyntheticInformation<typename Entry::KeyEntryInformationType>();
    original.setKeyEntryInformation(information);

    const ByteVector serialized = original.serializeKSTKeyEntry();
    const std::size_t keyLength = original.getLength();

    require(serialized.size() > keyLength, name + " serialized entry contains no metadata.");

    Entry restored;

    restored.deserializeKSTKeyEntryInformation(serialized.data() + keyLength, serialized.size() - keyLength, keyType);

    const auto &restoredInfo = restored.getKeyEntryInformation();
    const auto &originalInfo = original.getKeyEntryInformation();

    require(std::memcmp(restoredInfo.desfireAid, originalInfo.desfireAid, sizeof(originalInfo.desfireAid)) == 0,
            name + " DF_AID changed during metadata round-trip.");

    requireEqual(restoredInfo.desfirekeyno, originalInfo.desfirekeyno,
                 name + " desfirekeyno changed during metadata round-trip.");
    requireEqual(restoredInfo.cekno, originalInfo.cekno,
                 name + " cekno changed during metadata round-trip.");
    requireEqual(restoredInfo.cekv, originalInfo.cekv,
                 name + " cekv changed during metadata round-trip.");
    requireEqual(restoredInfo.kuc, originalInfo.kuc,
                 name + " kuc changed during metadata round-trip.");

    require(std::memcmp(restoredInfo.set, originalInfo.set, sizeof(originalInfo.set)) == 0,
            name + " SET changed during metadata round-trip.");

    requireEqual(restoredInfo.vera, originalInfo.vera,
                 name + " vera changed during metadata round-trip.");

    if (original.getKeyNb() >= 2)
    {
        requireEqual(restoredInfo.verb, originalInfo.verb, name + " verb changed during metadata round-trip.");
    }
    else
    {
        requireEqual(restoredInfo.verb, static_cast<unsigned char>(0), name + " verb should be zero for a 1 key entry.");
    }

    if (original.getKeyNb() >= 3)
    {
        requireEqual(restoredInfo.verc, originalInfo.verc,
                     name + " verc changed during metadata round-trip.");
    }
    else
    {
        requireEqual(restoredInfo.verc, static_cast<unsigned char>(0),
                     name + " verc should be zero for an entry with fewer than 3 keys.");
    }

    checkMetadataExtra(restoredInfo, originalInfo, name);

    std::cout << " [METADATA] " << name << " : OK\n";
}

// =================================================
// Malformed KST tests
// =================================================

template <typename Entry>
void testMalformedKST(const std::string &name, logicalaccess::SAMKeyType keyType)
{
    Entry original;

    initialiseSyntheticKeyMaterial(original, keyType);

    const auto information = makeSyntheticInformation<typename Entry::KeyEntryInformationType>();
    original.setKeyEntryInformation(information);

    const ByteVector serialized = original.serializeKSTKeyEntry();
    require(!serialized.empty(), name + " generated no serialized data");

    // Truncated input
    if (serialized.size() > 1)
    {
        ByteVector truncated = serialized;
        truncated.pop_back();
        requireThrows(name + " truncated KST",
                      [&]()
                      {
                          Entry restored;
                          restored.deserializeKSTKeyEntry(truncated.data(), truncated.size(), keyType);
                      });
    }

    // Trailing garbage
    {
        ByteVector withTrailingData = serialized;
        withTrailingData.push_back(0xEE);
        requireThrows(name + " trailing KST bytes",
                      [&]()
                      {
                          Entry restored;
                          restored.deserializeKSTKeyEntry(withTrailingData.data(), withTrailingData.size(), keyType);
                      });
    }

    // Null pointer with non-zero size
    requireThrows(name + " null KST buffer",
                  [&]()
                  {
                      Entry restored;
                      restored.deserializeKSTKeyEntry(nullptr, serialized.size(), keyType);
                  });

    // Wrong key type (choose another valid key type)
    logicalaccess::SAMKeyType wrongType = logicalaccess::SAMKeyType::SAM_KEY_AES256;

    if (keyType == logicalaccess::SAMKeyType::SAM_KEY_AES256)
        wrongType = logicalaccess::SAMKeyType::SAM_KEY_AES128;

    requireThrows(name + " wrong key type",
                  [&]()
                  {
                      Entry restored;
                      restored.deserializeKSTKeyEntry(serialized.data(), serialized.size(), wrongType);
                  });

    std::cout << " [MALFORMED INPUT] " << name << " : OK\n";
}

// =================================================
// SET public API tests
// =================================================

template <typename Entry, typename SET>
void testPublicSETAPI(const std::string &name)
{
    Entry entry;
    const SET expected = makeSyntheticSET<SET>();
    entry.setSET(expected);

    const SET actual = entry.getSETStruct();
    requireRawObjectEqual(actual, expected, name + " setSET/getSETStruct round-trip failed.");

    // Test raw SET setter
    const unsigned char rawSET[2] = {0xA5, 0x5A};
    entry.setSET(rawSET);

    const auto information = entry.getKeyEntryInformation();
    requireEqual(information.set[0], rawSET[0], name + " raw SET byte 0 was not stored.");
    requireEqual(information.set[1], rawSET[1], name + " raw SET byte 1 was not stored.");

    // Null pointer must be rejected
    requireThrows(name + " null SET", [&]() { entry.setSET(nullptr); });

    std::cout << " [PUBLIC SET API] " << name << " : OK\n";
}

// =================================================
// key-type / SET integration tests
// =================================================

template <typename Entry>
void testKeyTypeSynchronization(const std::string &name, logicalaccess::SAMKeyType keyType)
{
    Entry entry;
    initialiseSyntheticKeyMaterial(entry, keyType);

    const auto &information = entry.getKeyEntryInformation();

    // setKeysData() calls setSETKeyTypeFromKeyType()
    const unsigned char encodedKeyType = static_cast<unsigned char>(
        information.set[0] & logicalaccess::samav_detail::SAMKeyEntryKSTTraits<
                                 typename Entry::KeyEntryInformationType,
                                 typename Entry::SETType>::keyTypeMask);

    requireEqual(encodedKeyType, static_cast<unsigned char>(keyType), name + " key type was not synchronized into SET");

    // modify SET to contain the key type and verify extraction
    entry.setKeyTypeFromSET();
    requireEqual(entry.getKeyType(), keyType, name + " key type could not be recovered from SET");

    std::cout << " [KEY TYPE / SET] " << name << " : OK\n";
}

// =================================================
// Offline suite
// =================================================

void runOfflineTests()
{
    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "== OFFLINE SAMKeyEntry TESTS ==\n";
    std::cout << "== No reader. No SAM. No APDU. No key-writing operation. ==\n";
    std::cout << "=================================================\n";

    // -------------------------------------------------
    // Structure layout
    // -------------------------------------------------

    runSingleTest("SETAV1 layout", []() { requireSETStructureIsValid<logicalaccess::SETAV1>("SETAV1"); });
    runSingleTest("SETAV2 layout", []() { requireSETStructureIsValid<logicalaccess::SETAV2>("SETAV2"); });
    runSingleTest("SETAV3 layout", []() { requireSETStructureIsValid<logicalaccess::SETAV3>("SETAV3"); });

    // -------------------------------------------------
    // SET codec
    // -------------------------------------------------

    runSingleTest("SETAV1 encode/decode", []() { testSETRoundTrip<logicalaccess::SETAV1>("SETAV1"); });
    runSingleTest("SETAV2 encode/decode", []() { testSETRoundTrip<logicalaccess::SETAV2>("SETAV2"); });
    runSingleTest("SETAV3 encode/decode", []() { testSETRoundTrip<logicalaccess::SETAV3>("SETAV3"); });
    runSingleTest("SETAV1 individual-bit encoding", []() { testSETAllBits<logicalaccess::SETAV1>("SETAV1"); });
    runSingleTest("SETAV2 individual-bit encoding", []() { testSETAllBits<logicalaccess::SETAV2>("SETAV2"); });
    runSingleTest("SETAV3 individual-bit encoding", []() { testSETAllBits<logicalaccess::SETAV3>("SETAV3"); });

    // -------------------------------------------------
    // SAM AV1
    // -------------------------------------------------

    runSingleTest("SAMKeyEntry AV1 / 128-bit / 3 keys",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV1Information, logicalaccess::SETAV1>;
                      testSAMKeyEntryRoundTrip<Entry, logicalaccess::SETAV1>(
                          "AV1", logicalaccess::SAMKeyType::SAM_KEY_AES128);
                  });

    runSingleTest("SAMKeyEntry AV1 metadata",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV1Information, logicalaccess::SETAV1>;
            testMetadataRoundTrip<Entry>("AV1", logicalaccess::SAMKeyType::SAM_KEY_AES128);
        });

    runSingleTest("SAMKeyEntry AV1 malformed input",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV1Information, logicalaccess::SETAV1>;
            testMalformedKST<Entry>("AV1", logicalaccess::SAMKeyType::SAM_KEY_AES128);
        });

    runSingleTest("SAMKeyEntry AV1 SET API",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV1Information, logicalaccess::SETAV1>;
                      testPublicSETAPI<Entry, logicalaccess::SETAV1>("AV1");
                  });

    // -------------------------------------------------
    // SAM AV2
    // -------------------------------------------------

    runSingleTest("SAMKeyEntry AV2 / 192-bit / 2 keys",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;
                      testSAMKeyEntryRoundTrip<Entry, logicalaccess::SETAV2>(
                          "AV2", logicalaccess::SAMKeyType::SAM_KEY_AES192);
                  });

    runSingleTest("SAMKeyEntry AV2 metadata",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;
            testMetadataRoundTrip<Entry>("AV2", logicalaccess::SAMKeyType::SAM_KEY_AES192);
        });

    runSingleTest("SAMKeyEntry AV2 malformed input",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;
            testMalformedKST<Entry>("AV2", logicalaccess::SAMKeyType::SAM_KEY_AES192);
        });

    runSingleTest("SAMKeyEntry AV2 SET API",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;

                      testPublicSETAPI<Entry, logicalaccess::SETAV2>("AV2");
                  });

    // -------------------------------------------------
    // SAM AV3
    // -------------------------------------------------

    runSingleTest("SAMKeyEntry AV3 / 256-bit / 1 key",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV3Information, logicalaccess::SETAV3>;

                      testSAMKeyEntryRoundTrip<Entry, logicalaccess::SETAV3>(
                          "AV3", logicalaccess::SAMKeyType::SAM_KEY_AES256);
                  });

    runSingleTest("SAMKeyEntry AV3 metadata",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV3Information, logicalaccess::SETAV3>;
            testMetadataRoundTrip<Entry>("AV3", logicalaccess::SAMKeyType::SAM_KEY_AES256);
        });

    runSingleTest("SAMKeyEntry AV3 malformed input",
        []()
        {
            using Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV3Information, logicalaccess::SETAV3>;
            testMalformedKST<Entry>("AV3", logicalaccess::SAMKeyType::SAM_KEY_AES256);
        });

    runSingleTest("SAMKeyEntry AV3 SET API",
                  []()
                  {
                      using Entry = logicalaccess::SAMKeyEntry<
                          logicalaccess::KeyEntryAV3Information, logicalaccess::SETAV3>;

                      testPublicSETAPI<Entry, logicalaccess::SETAV3>("AV3");
                  });

    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "== OFFLINE TESTS COMPLETED ==\n";
    std::cout << "=================================================\n";
}

// =================================================
// Live SAM helpers
// =================================================

void printByteArray(const unsigned char *data, std::size_t size)
{
    std::cout << " HEX : ";

    for (std::size_t i = 0; i < size; ++i)
    {
        if (i != 0)
            std::cout << ' ';
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(data[i]);
    }

    std::cout << std::dec << std::setfill(' ') << '\n';
}

template <typename T>
void printSetStruct(const T &set)
{
    const auto *bytes = reinterpret_cast<const unsigned char *>(&set);
    std::cout << " Struct size : " << sizeof(T) << " bytes\n";
    std::cout << " Struct bytes :\n";

    printByteArray(bytes, sizeof(T));
    std::cout << " Active fields :\n";

    for (std::size_t byte = 0; byte < sizeof(T); ++byte)
    {
        for (std::size_t bit = 0; bit < 8; ++bit)
        {
            if ((bytes[byte] & (1U << bit)) != 0)
                std::cout << "  byte " << byte << ", bit " << bit << '\n';
        }
    }
}

// =================================================
// Live SAM key-entry test
// =================================================

struct LiveKeyEntryTestCase
{
    std::uint8_t keyNo;
};

template <typename Entry>
void printVersionSpecificInformation(const Entry &)
{
    // SAM AV1 has no version specific extra KST metadata
}

void printVersionSpecificInformation(const logicalaccess::KeyEntryAV2Information &info)
{
    std::cout << "\n[AV2 EXTSET]\n";
    std::cout << " ExtSET      : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(info.ExtSET) << std::dec
              << std::setfill(' ') << '\n';
}

void printVersionSpecificInformation(const logicalaccess::KeyEntryAV3Information &info)
{
    std::cout << "\n[AV3 EXTSET]\n";
    std::cout << " ExtSET      : 0x" << std::hex << std::uppercase << std::setw(4)
              << std::setfill('0') << static_cast<unsigned int>(info.ExtSET) << '\n';
    std::cout << " keyNoAEK    : 0x" << std::setw(2) << std::setfill('0')
              << static_cast<unsigned int>(info.keyNoAEK) << '\n';
    std::cout << " keyVerAEK   : 0x" << std::setw(2) << std::setfill('0')
              << static_cast<unsigned int>(info.keyVerAEK) << std::dec << std::setfill(' ') << '\n';
}

void runLiveSAMKeyEntryTest(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd, std::uint8_t keyNo)
{
    require(static_cast<bool>(samCmd), "SAM command object is null");

    std::cout << "\n[READ-ONLY] SAM_GetKeyEntry\n";
    std::cout << " KeyNo : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(keyNo) << std::dec << std::setfill(' ') << '\n';

    // =================================================
    // Physical SAM key entry operation is set here
    // =================================================
    
    // getKeyEntry() is read only : no key slot is overwritten
    auto keyEntry = samCmd->getKeyEntry(keyNo);

    require(static_cast<bool>(keyEntry), "SAM_GetKeyEntry returned null.");

    std::cout << "\n[KEY ENTRY]\n";
    std::cout << " Key number : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(keyEntry->getKeyNb())
              << std::dec << std::setfill(' ') << '\n';
    std::cout << " Key length : " << keyEntry->getLength() << " bytes\n";
    std::cout << " Key count  : " << static_cast<unsigned int>(keyEntry->getKeyNb()) << '\n';
    std::cout << " Key type   : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(keyEntry->getKeyType())
              << std::dec << std::setfill(' ') << '\n';

    // -------------------------------------------------
    // KST information
    // -------------------------------------------------

    const auto &info = keyEntry->getKeyEntryInformation();

    std::cout << "\n[KST INFORMATION]\n";
    std::cout << " DF_AID      : ";

    printByteArray(info.desfireAid, sizeof(info.desfireAid));

    std::cout << " DF_KeyNo    : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(info.desfirekeyno)
              << std::dec << std::setfill(' ') << '\n';
    std::cout << " KeyNoCEK    : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(info.cekno) << '\n';
    std::cout << " KeyVerCEK   : 0x" << std::setw(2) << static_cast<unsigned int>(info.cekv) << '\n';
    std::cout << " RefNoKUC    : 0x" << std::setw(2)
              << static_cast<unsigned int>(info.kuc) << std::dec << std::setfill(' ') << '\n';

    // -------------------------------------------------
    // SET raw
    // -------------------------------------------------

    std::cout << "\n[SET RAW]\n";
    std::cout << " SET[0]      : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(info.set[0]) << '\n';
    std::cout << " SET[1]      : 0x" << std::setw(2)
              << static_cast<unsigned int>(info.set[1]) << std::dec << std::setfill(' ') << '\n';

    // -------------------------------------------------
    // SET decoding
    // -------------------------------------------------

    std::cout << "\n[SET STRUCT]\n";

    const auto set = keyEntry->getSETStruct();
    printSetStruct(set);

    // -------------------------------------------------
    // Versions
    // -------------------------------------------------

    std::cout << "\n[KEY VERSIONS]\n";

    std::cout << " Version A   : 0x" << std::hex << std::uppercase << std::setw(2)
              << std::setfill('0') << static_cast<unsigned int>(info.vera) << '\n';
    std::cout << " Version B   : 0x" << std::setw(2) << static_cast<unsigned int>(info.verb) << '\n';
    std::cout << " Version C   : 0x" << std::setw(2)
              << static_cast<unsigned int>(info.verc) << std::dec << std::setfill(' ') << '\n';

    // -------------------------------------------------
    // Version specific extra metadata
    // -------------------------------------------------

    printVersionSpecificInformation(info);

    // -------------------------------------------------
    // SET encode/decode round-trip on REAL SAM data
    // -------------------------------------------------

    std::cout << "\n[REAL SAM SET ROUND-TRIP]\n";

    // Take the SET returned by the real SAM, convert it to the structured representation, and convert it back
    const auto decodedSET = keyEntry->getSETStruct();
    unsigned char encodedSET[2] = {0, 0};

    logicalaccess::samav_detail::encodeSET(decodedSET, encodedSET);
    requireEqual(encodedSET[0], info.set[0], "Real SAM SET byte 0 changed during encode/decode.");
    requireEqual(encodedSET[1], info.set[1], "Real SAM SET byte 1 changed during encode/decode.");

    std::cout << " Raw SET -> struct -> raw SET : OK\n";

    // -------------------------------------------------
    // KST complete serialization
    // -------------------------------------------------

    std::cout << "\n[SERIALIZED KST ENTRY]\n";

    /*
     * WARNING : serializeKSTKeyEntry() can contain the actual secret key material.
     * The serialization is executed and validated but the key material is never printed.
     * Only the non-secret metadata portion is shown
     */
    const ByteVector serialized = keyEntry->serializeKSTKeyEntry();
    require(!serialized.empty(), "serializeKSTKeyEntry() returned empty data.");

    const std::size_t kstKeyLength   = keyEntry->getLength();
    require(serialized.size() >= kstKeyLength, "Serialized KST entry is shorter than the key material.");

    const std::size_t metadataLength = serialized.size() - kstKeyLength;

    std::cout << " Size : " << serialized.size() << " bytes\n";
    std::cout << " Key material : " << kstKeyLength << " bytes [NOT PRINTED]\n";
    std::cout << " Metadata     : " << metadataLength << " bytes\n";
    std::cout << " HEX           : [KEY MATERIAL REDACTED] ";

    for (std::size_t i = kstKeyLength; i < serialized.size(); ++i)
    {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned int>(serialized[i]);

        if (i + 1 < serialized.size())
            std::cout << ' ';
    }

    std::cout << std::dec << std::setfill(' ') << '\n';
    std::cout << " Serialization : OK\n";

    // Don't print serialized bytes : they may contain secret key material

    // -------------------------------------------------
    // Complete deserialization round-trip
    // -------------------------------------------------

    std::cout << "\n[KST DESERIALIZATION ROUND-TRIP]\n";

    // getKeyEntry() returns a concrete SAMKeyEntry internally
    using AV2Entry = logicalaccess::SAMKeyEntry<logicalaccess::KeyEntryAV2Information, logicalaccess::SETAV2>;

    AV2Entry restored;
    restored.deserializeKSTKeyEntry(serialized.data(), serialized.size(), keyEntry->getKeyType());
    requireEqual(restored.getKeyType(), keyEntry->getKeyType(), "Deserialized key type differs from live SAM entry.");
    requireEqual(restored.getKeyNb(), keyEntry->getKeyNb(), "Deserialized key count differs from live SAM entry.");
    requireEqual(restored.getLength(), keyEntry->getLength(), "Deserialized key length differs from live SAM entry.");

    const auto &restoredInfo = restored.getKeyEntryInformation();
    require(std::memcmp(&restoredInfo, &info, sizeof(restoredInfo)) == 0,
            "Deserialized KST metadata differs from live SAM metadata.");

    const auto restoredSET = restored.getSETStruct();

    const unsigned char *restoredBytes = reinterpret_cast<const unsigned char *>(&restoredSET);
    const unsigned char *originalBytes = reinterpret_cast<const unsigned char *>(&set);

    if (std::memcmp(restoredBytes, originalBytes, sizeof(restoredSET)) != 0)
        throw std::runtime_error("Deserialized SET differs from live SAM SET.");

    // Re-serialize the deserialized object : it must be byte-identical to the original serialization
    const ByteVector reserialized = restored.serializeKSTKeyEntry();

    requireEqual(reserialized, serialized, "Live SAM KST serialization is not byte stable.");

    std::cout << " serialize -> deserialize -> serialize : OK\n";

    // -------------------------------------------------
    // Metadata-only deserialization
    // -------------------------------------------------

    std::cout << "\n[KST METADATA DESERIALIZATION]\n";

    const std::size_t keyLength = keyEntry->getLength();
    require(serialized.size() > keyLength, "Serialized KST entry contains no metadata");

    AV2Entry metadataOnly;
    metadataOnly.deserializeKSTKeyEntryInformation(serialized.data() + keyLength,
                                                   serialized.size() - keyLength,
                                                   keyEntry->getKeyType());

    const auto &metadataInfo = metadataOnly.getKeyEntryInformation();

    require(std::memcmp(&metadataInfo, &info, sizeof(metadataInfo)) == 0,
            "Metadata only deserialization differs from live SAM metadata.");

    std::cout << " Metadata only deserialization : OK\n";

    // -------------------------------------------------
    // Final
    // -------------------------------------------------
    std::cout << "\n[READ-ONLY SAM TEST] OK.\n";
    std::cout << "[SAFETY] No SAM key writing command was executed.\n";
}

// =================================================
// Reader session
// =================================================

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
        if (!reader)
            throw std::runtime_error("Failed to create reader unit");

        reader->setCardType("SAM_AV3");
        reader->connectToReader();
    }

    void waitCard()
    {
        if (!reader->waitInsertion(15000))
            throw std::runtime_error("No SAM inserted within timeout.");
        if (!reader->connect())
            throw std::runtime_error("Failed to connect to SAM.");

        std::cout << "Card inserted on reader : " << reader->getConnectedName() << '\n';

        chip = reader->getSingleChip();

        if (!chip)
            throw std::runtime_error("No chip detected.");
    }

    void waitRemovalSafe()
    {
        if (!reader->waitRemoval(3000))
            std::cerr << "[WARN] Card removal timeout.\n";
    }
};

// =================================================
// SAM session
// =================================================

struct SamSession
{
    std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd;
    std::shared_ptr<logicalaccess::DESFireKey> hostKey;

    void attach(const std::shared_ptr<logicalaccess::Chip> &chip)
    {
        if (!chip)
            throw std::runtime_error("Cannot attach SAM session: chip is null.");

        auto commands = chip->getCommands();
        if (!commands)
            throw std::runtime_error("Chip has no commands object.");

        std::cout << "[INFO] Commands type : " << typeid(*commands).name() << '\n';

        samCmd = std::dynamic_pointer_cast<logicalaccess::SAMAV3ISO7816Commands>(commands);
        if (!samCmd)
            throw std::runtime_error("SAMAV3ISO7816Commands not available.");
    }

    void authenticate()
    {
        if (!hostKey)
        {
            hostKey = std::make_shared<logicalaccess::DESFireKey>();
            hostKey->setKeyType(logicalaccess::DF_KEY_AES);

            // Host authentication key supplied to the host-side SAM authentication operation.
            hostKey->setData(logicalaccess::BufferHelper::fromHexString("00000000000000000000000000000000"));
        }

        samCmd->SAMAV2ISO7816Commands::authenticateHost(hostKey, 0x00);
    }
};

// =================================================
// Live SAM suite
// =================================================

void runLiveTests(std::shared_ptr<logicalaccess::SAMAV3ISO7816Commands> samCmd)
{
    require(static_cast<bool>(samCmd), "Cannot run live tests: SAM command object is null");

    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "== LIVE SAM READ-ONLY TESTS ==\n";
    std::cout << "== Physical SAM is accessed ==\n";
    std::cout << "== No key-writing command is permitted ==\n";
    std::cout << "=================================================\n";

    for (const std::uint8_t keyNo : LIVE_KEY_SLOTS)
    {
        try
        {
            runLiveSAMKeyEntryTest(samCmd, keyNo);
        }
        catch (const std::exception &e)
        {
            std::cerr << "\n[FAIL] Live key slot 0x" << std::hex << std::uppercase
                      << std::setw(2) << std::setfill('0')
                      << static_cast<unsigned int>(keyNo) << std::dec << std::setfill(' ')
                      << " : " << e.what() << '\n';
            throw;
        }
    }

    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "== LIVE READ-ONLY TESTS COMPLETED ==\n";
    std::cout << "=================================================n";
}

} // namespace

// =================================================
// Main
// =================================================

int main(int, char **)
{
    try
    {
        // Phase 1 : Pure offline tests
        // No reader/SAM/APDU/card required
        runOfflineTests();

        // Phase 2 : real SAM integration test
        auto readerConfig = std::make_shared<logicalaccess::ReaderConfiguration>();

        const std::string providerName = "PCSC";

        readerConfig->setReaderProvider(logicalaccess::LibraryManager::getInstance()->getReaderProvider(providerName));

        if (!readerConfig->getReaderProvider())
            throw std::runtime_error("No PCSC reader provider available.");

        auto readers = readerConfig->getReaderProvider()->getReaderList();

        if (readers.empty())
            throw std::runtime_error("No PCSC readers detected.");

        readerConfig->setReaderUnit(readers.at(0));

        ReaderSession readerSession;
        SamSession samSession;

        readerSession.readerConfig = readerConfig;

        // Wait for SAM insertion
        std::cout << "\nWaiting 15 seconds for SAM insertion...\n";

        readerSession.connect();
        readerSession.waitCard();

        std::cout << "[INFO] Card type : " << readerSession.chip->getCardType() << '\n';


        // Attach SAM commands
        samSession.attach(readerSession.chip);

        // Host authentication
        std::cout << "\n[AUTH] Performing SAM Host Authentication...\n";

        samSession.authenticate();

        std::cout << "[AUTH] OK\n";

        // Optional re-authentication hook
        TestHooks hooks;

        hooks.afterEach = [&]()
        {
            if (!REAUTH_AFTER_EACH_TEST)
                return;

            std::cout << "\n[HOOK] Re-authenticating SAM...\n";

            samSession.authenticate(); // No key writing operation occurs here

            std::cout << "[HOOK] SAM authenticated.\n";
        };

        // Tests on physical SAM
        runLiveTests(samSession.samCmd);

        // Removal
        std::cout << "\nWaiting for SAM removal...\n";

        readerSession.waitRemovalSafe();

        std::cout << "SAM removed.\n";
        std::cout << "\n=================================================\n";
        std::cout << "== ALL READ-ONLY SAMKeyEntry TESTS PASSED ==\n";
        std::cout << "=================================================\n";

        return EXIT_SUCCESS;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "\n[FATAL] " << ex.what() << '\n';
        std::cerr << "[SAFETY] This test program contains no SAM key-writing command.\n";

        return EXIT_FAILURE;
    }
}