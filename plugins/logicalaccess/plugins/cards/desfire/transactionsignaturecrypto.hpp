#pragma once

#include <openssl/types.h>

#include <logicalaccess/plugins/cards/desfire/transactioncommon.hpp>
#include <logicalaccess/plugins/cards/desfire/transactionmacinput.hpp>

namespace logicalaccess
{

class LLA_CARDS_DESFIRE_API DESFireTransactionSignatureCrypto
{
  public:
    using TransactionCounter        = TransactionManagement::TransactionCounter;
    using TransactionSignatureValue = TransactionManagement::TransactionSignatureValue;

    [[nodiscard]] static bool verifyTSV(const ByteVector &tmi, const TransactionCounter &committedTMC,
                          const TransactionSignatureValue &tsv, const EVP_PKEY *publicKey);
};

} // namespace logicalaccess