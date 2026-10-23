#pragma once

#include <array>
#include <cstdint>

#include <openssl/types.h>

#include <logicalaccess/plugins/cards/desfire/transactionmacinput.hpp>

namespace logicalaccess
{

class LLA_CARDS_DESFIRE_API DESFireTransactionSignatureCrypto
{
  public:
    static constexpr std::size_t TMC_SIZE = 4U;
    static constexpr std::size_t TSV_SIZE = 64U;

    using TransactionCounter        = std::array<std::uint8_t, TMC_SIZE>;
    using TransactionSignatureValue = std::array<std::uint8_t, TSV_SIZE>;

    [[nodiscard]] static bool verifyTSV(const ByteVector &tmi, const TransactionCounter &committedTMC,
                          const TransactionSignatureValue &tsv, const EVP_PKEY *publicKey);
};

} // namespace logicalaccess