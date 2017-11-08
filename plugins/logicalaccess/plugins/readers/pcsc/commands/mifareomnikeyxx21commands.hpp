//
// Created by xaqq on 6/22/15.
//

#ifndef LIBLOGICALACCESS_MIFAREOMNIKEYXX21COMMANDS_HPP
#define LIBLOGICALACCESS_MIFAREOMNIKEYXX21COMMANDS_HPP

#include <logicalaccess/plugins/readers/pcsc/commands/mifarepcsccommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_MIFAREOMNIKEYXX21 "MifareOmnikeyXX21"

/**
 * \brief The Mifare commands class for Omnikey xx27 reader.
 */
class LIBLOGICALACCESS_API MifareOmnikeyXX21Commands : public MifarePCSCCommands
{
  public:
    /**
     * \brief Constructor.
     */
    MifareOmnikeyXX21Commands();

    explicit MifareOmnikeyXX21Commands(std::string);

    /**
     * \brief Destructor.
     */
    virtual ~MifareOmnikeyXX21Commands();

    void increment(uint8_t blockno, uint32_t value) override;

    void decrement(uint8_t blockno, uint32_t value) override;
};
}

#endif // LIBLOGICALACCESS_MIFAREOMNIKEYXX21COMMANDS_H
