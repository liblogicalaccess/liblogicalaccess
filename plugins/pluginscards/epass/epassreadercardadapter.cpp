#include "epassreadercardadapter.hpp"
#include "epasscrypto.hpp"
#include <cassert>
using namespace logicalaccess;

EPassReaderCardAdapter::EPassReaderCardAdapter()
{
}

ByteVector EPassReaderCardAdapter::adaptAnswer(const ByteVector &answer)
{
    ByteVector r;
    if (crypto_ && crypto_->secureMode())
        r = crypto_->decrypt_rapdu(answer);
    else
        r = answer;
    // ResultChecker already ran, we can clear the SW1SW2 flags.
    if (r.size() > 2)
        return ByteVector(r.begin(), r.end() - 2);

    return r;
}

ByteVector EPassReaderCardAdapter::adaptCommand(const ByteVector &command)
{
    if (crypto_ && crypto_->secureMode())
        return crypto_->encrypt_apdu(command);
    return command;
}

void EPassReaderCardAdapter::setEPassCrypto(std::shared_ptr<EPassCrypto> crypto)
{
    crypto_ = crypto;
}
