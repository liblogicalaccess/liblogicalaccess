/**
 * \file samkeyentry.hpp
 * \author Adrien J. <adrien.jund@islog.com>
 * \brief samkeyentry header.
 */

#ifndef LOGICALACCESS_SAMKEYENTRY_HPP
#define LOGICALACCESS_SAMKEYENTRY_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/samav/sambasickeyentry.hpp>

namespace logicalaccess
{

// ----------------------------------------------
// SET configuration structures
// ----------------------------------------------

struct SETAV1
{
    unsigned char dumpsessionkey;
    unsigned char allowcrypto;
    unsigned char keepIV;
    unsigned char keytype[3];
    unsigned char rfu[2];
    unsigned char hightcommandsecurity;
    unsigned char disablekeyentry;
    unsigned char hostauthenticationafterreset;
    unsigned char disablewritekeytopicc;
    unsigned char disabledecryption;
    unsigned char disableencryption;
    unsigned char disableverifymac;
    unsigned char disablegeneratemac;
};

struct SETAV2
{
    unsigned char dumpsessionkey;
    unsigned char allowcrypto;
    unsigned char keepIV;
    unsigned char keytype[3];
    unsigned char rfu[2];
    unsigned char authkey;
    unsigned char disablekeyentry;
    unsigned char lockkey;
    unsigned char disablewritekeytopicc;
    unsigned char disabledecryption;
    unsigned char disableencryption;
    unsigned char disableverifymac;
    unsigned char disablegeneratemac;
};

struct SETAV3
{
    unsigned char dumpsessionkey;
    unsigned char rfu;
    unsigned char keepIV;
    unsigned char keytype[4];
    unsigned char plkey;
    unsigned char authkey;
    unsigned char disablekeyentry;
    unsigned char lockkey;
    unsigned char disablewritekeytopicc;
    unsigned char disabledecryption;
    unsigned char disableencryption;
    unsigned char disableverifymac;
    unsigned char disablegeneratemac;
};

// ----------------------------------------------
// ExtSET configuration structures
// ----------------------------------------------

struct BaseExtSETStruct
{
    unsigned char keyclass[3];
    unsigned char dumpsecretkey;
    unsigned char diversifieduse;
    unsigned char reservedforperso;
};

struct ExtSETStruct : BaseExtSETStruct
{
    unsigned char rfu;
};

struct ExtSETAV3 : BaseExtSETStruct
{
    unsigned char rfu[2];
    unsigned char forcekeyusageinternalhost;
    unsigned char forcekeychangeinternalhost;
    unsigned char forcesessionusageinternalhost;
    unsigned char dumpsecretkeyinternalhost;
    unsigned char dumpsessionkeyinternalhost;
    unsigned char rfu2[3];
};

// ----------------------------------------------
// KST key entry information
// ----------------------------------------------

struct KeyEntryInformationBase
{
    unsigned char desfireAid[3];
    unsigned char desfirekeyno;
    unsigned char cekno;
    unsigned char cekv;
    unsigned char kuc;
    unsigned char set[2];
    unsigned char vera;
    unsigned char verb;
    unsigned char verc;
};

struct KeyEntryAV1Information : KeyEntryInformationBase
{
};

struct KeyEntryAV2Information : KeyEntryInformationBase
{
    unsigned char ExtSET;
};

struct KeyEntryAV3Information : KeyEntryInformationBase
{
    std::uint16_t ExtSET;
    unsigned char keyNoAEK;
    unsigned char keyVerAEK;
};

// ----------------------------------------------
// SAM implementation details
// ----------------------------------------------

namespace samav_detail
{

// ----------------------------------------------
// KST byte writer
// ----------------------------------------------

class KSTByteWriter
{
  public:
    explicit KSTByteWriter(ByteVector &output)
        : d_output(output)
    {
    }

    void writeByte(unsigned char value)
    {
        d_output.push_back(value);
    }

    void writeBytes(const unsigned char *data, std::size_t size)
    {
        if (size == 0)
            return;

        if (data == nullptr)
            throw std::invalid_argument("Cannot serialize null KST data.");

        d_output.insert(d_output.end(), data, data + size);
    }

    void writeLittleEndian16(std::uint16_t value)
    {
        writeByte(static_cast<unsigned char>(value & 0xFFU));
        writeByte(static_cast<unsigned char>((value >> 8U) & 0xFFU));
    }

  private:
    ByteVector &d_output;
};

// ----------------------------------------------
// KST byte reader
// ----------------------------------------------
class KSTByteReader
{
  public:
    KSTByteReader(const unsigned char *data, std::size_t size)
        : d_data(data)
        , d_size(size)
    {
        if (data == nullptr && size != 0)
            throw std::invalid_argument("Cannot deserialize from null KST data.");
    }

    unsigned char readByte()
    {
        ensureAvailable(1);
        return d_data[d_offset++];
    }

    void readBytes(unsigned char *destination, std::size_t size)
    {
        if (size == 0)
            return;

        if (destination == nullptr)
            throw std::invalid_argument("Cannot deserialize KST data into null destination.");

        ensureAvailable(size);
        std::memcpy(destination, d_data + d_offset, size);
        d_offset += size;
    }

    std::uint16_t consumeUint16LE()
    {
        ensureAvailable(2);

        const std::uint16_t value =
            static_cast<std::uint16_t>(d_data[d_offset]) |
            static_cast<std::uint16_t>(static_cast<std::uint16_t>(d_data[d_offset + 1]) << 8U);

        d_offset += 2;

        return value;
    }

    std::size_t bytesRemaining() const noexcept
    {
        return d_size - d_offset;
    }

  private:
    void ensureAvailable(std::size_t count) const
    {
        if (count > bytesRemaining())
            throw std::out_of_range("Unexpected end of SAM KST key entry.");
    }

    const unsigned char *d_data;
    std::size_t d_size;
    std::size_t d_offset = 0;
};

// ----------------------------------------------
// SET encoding/decoding
// ----------------------------------------------

template <typename S>
S decodeSET(const unsigned char (&setBytes)[2])
{
    static_assert(sizeof(S) == 16, "SAM SET structure must contain exactly 16 bytes.");

    S set{};
    auto *const output = reinterpret_cast<unsigned char *>(&set);

    for (std::size_t byte = 0; byte < sizeof(setBytes); ++byte)
    {
        const unsigned char value = setBytes[byte];

        for (std::size_t bit = 0; bit < 8; ++bit)
            output[byte * 8U + bit] = static_cast<unsigned char>((value >> bit) & 0x01U);
    }

    return set;
}

template <typename S>
void encodeSET(const S &set, unsigned char (&setBytes)[2])
{
    static_assert(sizeof(S) == 16, "SAM SET structure must contain exactly 16 bytes.");

    const auto *const input = reinterpret_cast<const unsigned char *>(&set);

    for (std::size_t byte = 0; byte < sizeof(setBytes); ++byte)
    {
        unsigned char value = 0;

        for (std::size_t bit = 0; bit < 8; ++bit)
            value = static_cast<unsigned char>((value << 1U) | (input[byte * 8U + (7U - bit)] & 0x01U));

        setBytes[byte] = value;
    }
}

// ----------------------------------------------
// Version specific KST traits
// ----------------------------------------------

template <typename T, typename S>
struct SAMKeyEntryKSTTraits
{
    static constexpr std::size_t serializedExtraSize = 0;
    static constexpr unsigned char keyTypeMask = 0x38U;

    static void serializeExtra(KSTByteWriter &, const T &) {}

    static void deserializeExtra(KSTByteReader &, T &) {}
};

// ----------------------------------------------
// SAM AV1
// ----------------------------------------------

template <>
struct SAMKeyEntryKSTTraits<KeyEntryAV1Information, SETAV1>
{
    static constexpr std::size_t serializedExtraSize = 0;
    static constexpr unsigned char keyTypeMask       = 0x38U; // AV1 key type occupies SET bits 3 to 5

    static void serializeExtra(KSTByteWriter &, const KeyEntryAV1Information &) {}

    static void deserializeExtra(KSTByteReader &, KeyEntryAV1Information &) {}
};

// ----------------------------------------------
// SAM AV2
// ----------------------------------------------

template <>
struct SAMKeyEntryKSTTraits<KeyEntryAV2Information, SETAV2>
{
    static constexpr std::size_t serializedExtraSize = 1;
    static constexpr unsigned char keyTypeMask       = 0x38U; // AV2 key type occupies SET bits 3 to 5

    static void serializeExtra(KSTByteWriter &writer, const KeyEntryAV2Information &information)
    {
        writer.writeByte(information.ExtSET);
    }

    static void deserializeExtra(KSTByteReader &reader, KeyEntryAV2Information &information)
    {
        information.ExtSET = reader.readByte();
    }
};

// ----------------------------------------------
// SAM AV3
// ----------------------------------------------

template <>
struct SAMKeyEntryKSTTraits<KeyEntryAV3Information, SETAV3>
{
    static constexpr std::size_t serializedExtraSize = 4;
    static constexpr unsigned char keyTypeMask       = 0x78U; // AV3 key type occupies SET bits 3 to 6

    static void serializeExtra(KSTByteWriter &writer, const KeyEntryAV3Information &information)
    {
        // SAM AV3 serializes ExtSET LSB first.
        writer.writeLittleEndian16(information.ExtSET);
        writer.writeByte(information.keyNoAEK);
        writer.writeByte(information.keyVerAEK);
    }

    static void deserializeExtra(KSTByteReader &reader, KeyEntryAV3Information &information)
    {
        // SAM AV3 serializes ExtSET LSB first.
        information.ExtSET = reader.consumeUint16LE();
        information.keyNoAEK = reader.readByte();
        information.keyVerAEK = reader.readByte();
    }
};

} // namespace samav_detail

// ----------------------------------------------
// SAMKeyEntry
// ----------------------------------------------

/**
 * \brief A SAMKeyEntry class.
 * T is the version specific KST key entry information structure.
 * S is the corresponding 16 byte SET structure where every bit is represented by one byte containing 0 or 1.
 */
template <typename T, typename S>
class LLA_CARDS_SAMAV_API SAMKeyEntry : public SAMBasicKeyEntry
{
    
  public:
    SAMKeyEntry()
        : SAMBasicKeyEntry()
        , d_keyentryinformation{}
    {
    }

    explicit SAMKeyEntry(const std::string &str, const std::string &str1 = "", const std::string &str2 = "")
        : SAMBasicKeyEntry(str, str1, str2)
        , d_keyentryinformation{}
    {
    }

    SAMKeyEntry(const void **buf, std::size_t buflen, char numberkey)
        : SAMBasicKeyEntry(buf, buflen, numberkey)
        , d_keyentryinformation{}
    {
    }

    /**
     * \brief Destructor.
     */
    ~SAMKeyEntry() override = default;

    /**
     * \brief Inequality operator
     * \param ai SAMKeyEntry key to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const SAMKeyEntry &key) const
    {
        return !operator==(key);
    }

    void setSET(const unsigned char *value)
    {
        if (value == nullptr)
            throw std::invalid_argument("SET value must not be null.");

        std::memcpy(d_keyentryinformation.set, value, sizeof(d_keyentryinformation.set));
    }

    /**
     * \brief Decode the raw SAM SET into the version specific SET structure.
     */
    S getSETStruct() const
    {
        return samav_detail::decodeSET<S>(d_keyentryinformation.set);
    }

    /**
     * \brief Encode a version specific SET structure into the raw SAM SET.
     */
    void setSET(const S &value)
    {
        samav_detail::encodeSET<S>(value, d_keyentryinformation.set);
    }

    T &getKeyEntryInformation()
    {
        return d_keyentryinformation;
    }

    const T &getKeyEntryInformation() const
    {
        return d_keyentryinformation;
    }

    void setKeyEntryInformation(const T &value)
    {
        d_keyentryinformation = value;
    }

    /**
     * \brief Extract the key type from SET.
     *
     * AV1/AV2 use bits 3 to 5 (0x38).
     * AV3 uses bits 3 to 6 (0x78).
     */
    void setKeyTypeFromSET()
    {
        const unsigned char keyType = static_cast<unsigned char>(d_keyentryinformation.set[0] & Traits::keyTypeMask);
        d_keyType = static_cast<SAMKeyType>(keyType);
    }

    /**
     * \brief Write the current SAM key type into SET.
     */
    void setSETKeyTypeFromKeyType()
    {
        constexpr unsigned char mask = Traits::keyTypeMask;

        d_keyentryinformation.set[0] = static_cast<unsigned char>(
            (d_keyentryinformation.set[0] & static_cast<unsigned char>(~mask)) |
            (static_cast<unsigned char>(d_keyType) & mask));
    }

    void setKeysData(std::vector<ByteVector> keys, SAMKeyType type) override
    {
        SAMBasicKeyEntry::setKeysData(keys, type);
        setSETKeyTypeFromKeyType();
    }

    ByteVector serializeKSTKeyEntry() const override
    {
        const std::size_t keyLength      = getLength();
        const std::size_t keyCount       = getKeyNb();
        const std::size_t metadataLength = serializedMetadataSize(keyCount);

        ByteVector entry;
        entry.reserve(keyLength + metadataLength);

        // KeyVa, KeyVb, KeyVc
        entry.insert(entry.end(), d_key, d_key + keyLength);
        // Common KST metadata
        samav_detail::KSTByteWriter writer(entry);
        serializeCommonMetadata(writer);
        // Version specific metadata
        Traits::serializeExtra(writer, d_keyentryinformation);

        return entry;
    }

    /**
     * \brief Deserialize only KST metadata.
     *
     * This expects data to start at DF_AID (immediately after the key material).
     */
    void deserializeKSTKeyEntryInformation(const unsigned char *data, std::size_t size, SAMKeyType expectedKeyType)
    {
        if (data == nullptr && size != 0)
            throw std::invalid_argument("KST metadata buffer must not be null.");

        d_keyType = expectedKeyType;

        samav_detail::KSTByteReader reader(data, size);
        deserializeCommonMetadata(reader);

        Traits::deserializeExtra(reader, d_keyentryinformation);

        if (reader.bytesRemaining() != 0)
            throw std::invalid_argument("Unexpected trailing bytes in SAM KST metadata.");
    }

    /**
     * \brief Deserialize a complete KST key entry.
     */
    void deserializeKSTKeyEntry(const unsigned char *data, std::size_t size, SAMKeyType expectedKeyType)
    {
        if (data == nullptr && size != 0)
            throw std::invalid_argument("KST key entry buffer must not be null.");

        SAMKeyEntry<T, S> temporary;
        temporary.d_keyType = expectedKeyType;
        const std::size_t keyLength = temporary.getLength();
        const std::size_t keyCount = temporary.getKeyNb();
        const std::size_t metadataLength = serializedMetadataSize(keyCount);
        const std::size_t expectedTotalSize = keyLength + metadataLength;

        if (size != expectedTotalSize)
            throw std::invalid_argument("Invalid SAM KST key entry size.");

        std::memcpy(temporary.d_key, data, keyLength);

        samav_detail::KSTByteReader reader(data + keyLength, metadataLength);
        temporary.deserializeCommonMetadata(reader);
        Traits::deserializeExtra(reader, temporary.d_keyentryinformation);

        if (reader.bytesRemaining() != 0)
            throw std::invalid_argument("Unexpected trailing bytes in SAM KST key entry.");

        const unsigned char encodedKeyType = static_cast<unsigned char>(
            temporary.d_keyentryinformation.set[0] & Traits::keyTypeMask);

        if (encodedKeyType != static_cast<unsigned char>(expectedKeyType))
            throw std::invalid_argument("SAM KST key type does not match the supplied key type.");

        d_keyType = expectedKeyType;

        std::memcpy(d_key, temporary.d_key, keyLength);

        if (keyLength < sizeof(d_key))
            std::memset(d_key + keyLength, 0, sizeof(d_key) - keyLength);

        d_keyentryinformation = temporary.d_keyentryinformation;
    }

    using KeyEntryInformationType = T;
    using SETType = S;

  private:
    using Traits = samav_detail::SAMKeyEntryKSTTraits<T, S>;
    static constexpr std::size_t COMMON_METADATA_SIZE = 12;

    // Number of version bytes present in the serialized metadata.
    static constexpr std::size_t serializedCommonMetadataSize(std::size_t keyCount)
    {
        return COMMON_METADATA_SIZE - ((keyCount < 2) ? 1U : 0U) - ((keyCount < 3) ? 1U : 0U);
    }

    static constexpr std::size_t serializedMetadataSize(std::size_t keyCount)
    {
        return serializedCommonMetadataSize(keyCount) + Traits::serializedExtraSize;
    }

    void serializeCommonMetadata(samav_detail::KSTByteWriter &writer) const
    {
        writer.writeBytes(d_keyentryinformation.desfireAid, sizeof(d_keyentryinformation.desfireAid));
        writer.writeByte(d_keyentryinformation.desfirekeyno);
        writer.writeByte(d_keyentryinformation.cekno);
        writer.writeByte(d_keyentryinformation.cekv);
        writer.writeByte(d_keyentryinformation.kuc);
        writer.writeBytes(d_keyentryinformation.set, sizeof(d_keyentryinformation.set));
        writer.writeByte(d_keyentryinformation.vera);
        if (getKeyNb() >= 2)
            writer.writeByte(d_keyentryinformation.verb);
        if (getKeyNb() >= 3)
            writer.writeByte(d_keyentryinformation.verc);
    }

    void deserializeCommonMetadata(samav_detail::KSTByteReader &reader)
    {
        reader.readBytes(d_keyentryinformation.desfireAid, sizeof(d_keyentryinformation.desfireAid));
        d_keyentryinformation.desfirekeyno = reader.readByte();
        d_keyentryinformation.cekno = reader.readByte();
        d_keyentryinformation.cekv = reader.readByte();
        d_keyentryinformation.kuc = reader.readByte();
        reader.readBytes(d_keyentryinformation.set, sizeof(d_keyentryinformation.set));
        d_keyentryinformation.vera = reader.readByte();
        if (getKeyNb() >= 2)
        {
            d_keyentryinformation.verb = reader.readByte();
        }
        else
        {
            d_keyentryinformation.verb = 0;
        }

        if (getKeyNb() >= 3)
        {
            d_keyentryinformation.verc = reader.readByte();
        }
        else
        {
            d_keyentryinformation.verc = 0;
        }
    }

    T d_keyentryinformation;
};

} // namespace logicalaccess

#endif /* LOGICALACCESS_SAMKEYENTRY_HPP */