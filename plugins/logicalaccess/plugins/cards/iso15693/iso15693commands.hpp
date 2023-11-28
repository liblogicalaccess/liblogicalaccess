/**
 * \file iso15693commands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO15693 commands.
 */

#ifndef LOGICALACCESS_ISO15693COMMANDS_HPP
#define LOGICALACCESS_ISO15693COMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/iso15693/iso15693location.hpp>
#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{
#define CMD_ISO15693 "ISO15693"

/**
 * \brief The ISO15693 commands class.
 */
class LLA_CARDS_ISO15693_API ISO15693Commands : public Commands
{
  public:
    ISO15693Commands()
        : Commands(CMD_ISO15693)
    {
    }

    explicit ISO15693Commands(std::string ct)
        : Commands(ct)
    {
    }

    /**
     * \brief ISO15693 System information.
     */
    struct SystemInformation
    {
        bool hasDSFID;             /**< \brief The DSFID is supported */
        bool hasAFI;               /**< \brief The AFI is supported */
        bool hasVICCMemorySize;    /**< \brief The VICC memory size is supported */
        bool hasICReference;       /**< \brief The IC reference is supported */
        unsigned char ICReference; /**< \brief The IC reference */
        int blockSize;             /**< \brief The block size in bytes */
        int nbBlocks;              /**< \brief The number of blocks */
        unsigned char AFI;         /**< \brief The Application Family Identifier */
        unsigned char DSFID;       /**< \brief The Data Storage Format Identifier */
    };

    virtual void stayQuiet() = 0;

    virtual ByteVector readBlock(size_t block, size_t le = 0) = 0;

    virtual void writeBlock(size_t block, const ByteVector &data) = 0;

    virtual void lockBlock(size_t block) = 0;

    virtual void writeAFI(size_t afi) = 0;

    virtual void lockAFI() = 0;

    virtual void writeDSFID(size_t dsfid) = 0;

    virtual void lockDSFID() = 0;

    virtual SystemInformation getSystemInformation() = 0;

    virtual unsigned char getSecurityStatus(size_t block) = 0;
};
}

#endif