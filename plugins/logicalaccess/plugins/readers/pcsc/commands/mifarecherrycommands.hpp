/**
 * \file mifarecherrycommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Cherry commands.
 */

#ifndef LOGICALACCESS_MIFARECHERRYCOMMANDS_HPP
#define LOGICALACCESS_MIFARECHERRYCOMMANDS_HPP

#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFARECHERRY "MifareCherry"
/**
 * \brief The Mifare commands class for Cherry reader.
 */
class LLA_READERS_PCSC_API MifareCherryCommands : public MifarePCSCCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareCherryCommands();

    explicit MifareCherryCommands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifareCherryCommands();

  protected:
    /**
     * \brief Load a key to the reader.
     * \param keyno The key number.
     * \param keytype The mifare key type.
     * \param key The key.
     * \param vol Use volatile memory.
     * \return true on success, false otherwise.
     */
    bool loadKey(unsigned char keyno, MifareKeyType keytype,
                 std::shared_ptr<MifareKey> key, bool vol = false) override;

    /**
     * \brief Authenticate a block, given a key number.
     * \param blockno The block number.
     * \param keyno The key number, previously loaded with Mifare::loadKey().
     * \param keytype The key type.
     */
    void authenticate(unsigned char blockno, unsigned char keyno,
                      MifareKeyType keytype) override;
};
}

#endif /* LOGICALACCESS_MIFARECHERRYCOMMANDS_HPP */