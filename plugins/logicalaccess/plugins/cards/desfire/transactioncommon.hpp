#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace logicalaccess
{

namespace TransactionManagement
{

// ---------------------------------------
// DESFire transaction protocol sizes
// ---------------------------------------

constexpr std::size_t AES_BLOCK_SIZE = 16U;

constexpr std::size_t TMC_SIZE = 4U;
constexpr std::size_t TMV_SIZE = 8U;
constexpr std::size_t TSV_SIZE = 64U;

constexpr std::size_t TMAC_SIZE = TMC_SIZE + TMV_SIZE;
constexpr std::size_t TSIG_SIZE = TMC_SIZE + TSV_SIZE;

constexpr std::size_t READER_ID_SIZE = AES_BLOCK_SIZE;

// ---------------------------------------
// Transaction data types
// ---------------------------------------

using TransactionCounter        = std::array<std::uint8_t, TMC_SIZE>;
using TransactionMACValue       = std::array<std::uint8_t, TMV_SIZE>;
using TransactionSignatureValue = std::array<std::uint8_t, TSV_SIZE>;
using ReaderID                  = std::array<std::uint8_t, READER_ID_SIZE>;

// ---------------------------------------
// AES key sizes
// ---------------------------------------

constexpr std::size_t AES_128_KEY_SIZE = 16U;
constexpr std::size_t AES_256_KEY_SIZE = 32U;

// ---------------------------------------
// DESFire command encoding
// ---------------------------------------

constexpr std::uint32_t U24_MAX = 0x00FFFFFFU;

} // namespace TransactionManagement

} // namespace logicalaccess