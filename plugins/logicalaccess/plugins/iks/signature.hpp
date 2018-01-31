#pragma once

#include "logicalaccess/iks/packet/iks.grpc.pb.h"
#include "logicalaccess/lla_fwd.hpp"

namespace logicalaccess
{
namespace iks
{
/**
 * An object describing a signature issued by IKS.
 *
 * The object contains both the original signed payload aswell as
 * signature parameter and the signature itself.
 */
struct SignatureResult
{

    bool verify(const std::string &pem_pubkey);

    ByteVector signature_;
    ::SignatureDescription signature_description_;
};
}
}
