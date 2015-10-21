#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>


namespace logicalaccess
{
class Key;
namespace iks
{

/**
 * Status code for a response.
 */
enum response_status_e : uint16_t
{
    /**
     * The operation was a success.
     */
    SMSG_STATUS_SUCCESS,

    /**
     * The operation failed, but we don't know much more.
     */
    SMSG_STATUS_FAILURE,

    /**
     * The payload size is invalid.
     *
     * This is probably because the payload doesn't
     * match the size of a block in a cryptographic operation.
     */
    SMSG_STATUS_INVALID_PAYLOAD_SIZE,

    /**
     * You requested too many bytes (for command_genrandom_t).
     */
    SMSG_STATUS_TOO_MANY_BYTES,

    SMSG_STATUS_CANNOT_LOAD_KEY,
    SMSG_STATUS_INVALID_FLAGS
};

/**
 * Returns a string representing the status code of a Response.
 */
std::string strstatus(enum response_status_e);
std::string strstatus(uint16_t st);

enum protocol_opcode
{
    CMSG_OP_AES_ENCRYPT,
    SMSG_OP_AES_ENCRYPT,
    CMSG_OP_GENRANDOM,
    SMSG_OP_GENRANDOM,
    CMSG_OP_DESFIRE_AUTH,
    SMSG_OP_DESFIRE_AUTH,
    CMSG_OP_DES_ENCRYPT,
    SMSG_OP_DES_ENCRYPT,
    CMSG_OP_DESFIRE_CHANGEKEY,
    SMSG_OP_DESFIRE_CHANGEKEY,
};

class BaseCommand
{
  public:
    virtual std::vector<uint8_t> serialize() const;

    /**
     * Compute the binary size_.
     */
    virtual size_t binary_size() const final;

    /**
     * Return the size_ of packet specific data.
     */
    virtual size_t binary_size_impl() const = 0;

  protected:
    uint16_t opcode_;
};

/**
 * Some information about how to do key diversification.
 */
class KeyDivInfo
{
  public:
    KeyDivInfo();

#define KEYDIV_ALGO_NONE 1 // No diversification required.
#define KEYDIV_ALGO_NXP_AV1 2
#define KEYDIV_ALGO_NXP_AV2 4
    uint8_t flag_;

    // We used fixed size so that it's easier network wise.
    // It will be truncated/extended to 64bytes.
    mutable std::vector<uint8_t> div_input_;

    size_t binary_size() const;
    std::vector<uint8_t> serialize() const;

    /**
     * Create a KeyDivInfo structure ready to send to the
     * remote IKS.
     *
     * `divinput` shall have been computed locally. Max length for
     * `divinput` is 64bytes.
     */
    static KeyDivInfo build(std::shared_ptr<Key> key,
                            const std::vector<uint8_t> &divinput);

    static KeyDivInfo build(std::shared_ptr<Key> key,
                            std::vector<unsigned char> identifier, int AID,
                            unsigned char keyno);
};

class BaseResponse
{
  public:
    virtual ~BaseResponse() = default;
    BaseResponse(uint16_t opcode, uint16_t status);

    uint16_t opcode_;
    uint16_t status_;
};
}
}
