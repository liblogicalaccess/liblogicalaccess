/**
 * \file mifarespringcardcommands.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief Mifare SpringCard card.
 */

#ifndef LOGICALACCESS_MIFARESPRINGCARDCOMMANDS_HPP
#define LOGICALACCESS_MIFARESPRINGCARDCOMMANDS_HPP

#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFARESPRINGCARD "MifareSpringCard"

/**
 * \brief The Mifare commands class for SpringCard reader.
 */
class LLA_READERS_PCSC_API MifareSpringCardCommands : public MifarePCSCCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareSpringCardCommands();

    explicit MifareSpringCardCommands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifareSpringCardCommands();

#ifndef SWIG
    using MifarePCSCCommands::authenticate;
    using MifarePCSCCommands::loadKey;
#endif

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

    /**
    * \brief Store block value to volatile memory.
    * \param blockno The block number.
    */
    virtual void restore(unsigned char blockno);

    /**
    * \brief Increment a block value.
    * \param blockno The block number.
    * \param value The increment value.
    */
    void increment(unsigned char blockno, uint32_t value) override;

    /**
    * \brief Decrement a block value.
    * \param blockno The block number.
    * \param value The decrement value.
    */
    void decrement(unsigned char blockno, uint32_t value) override;
};
}

#endif /* LOGICALACCESS_MIFARESPRINGCARDCOMMANDS_HPP */