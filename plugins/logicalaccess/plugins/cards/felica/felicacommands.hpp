/**
 * \file felicacommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa commands.
 */

#ifndef LOGICALACCESS_FELICACOMMANDS_HPP
#define LOGICALACCESS_FELICACOMMANDS_HPP

#include <logicalaccess/key.hpp>
#include <logicalaccess/plugins/cards/felica/felicalocation.hpp>
#include <logicalaccess/cards/commands.hpp>

namespace logicalaccess
{
#define CMD_FELICA "FeliCa"

/**
 * \brief The FeliCa commands class.
 */
class LLA_CARDS_FELICA_API FeliCaCommands : public Commands
{
  public:
    FeliCaCommands()
        : Commands(CMD_FELICA)
    {
    }

    explicit FeliCaCommands(std::string ct)
        : Commands(ct)
    {
    }

    /**
     * \brief Get system codes.
     * \return System codes list.
     */
    virtual std::vector<unsigned short> getSystemCodes() = 0;

    virtual unsigned short requestService(unsigned short code);

    /**
    * \brief Request Services / Area versions.
    * \param codes Service / Area codes list.
    * \return Service / Area versions.
    */
    virtual std::vector<unsigned short>
    requestServices(const std::vector<unsigned short> &codes) = 0;

    /**
    * \brief Request Response, current FeliCa mode.
    * \return FeliCa mode (0/1/2).
    */
    virtual unsigned char requestResponse() = 0;

    virtual ByteVector read(unsigned short code, unsigned short block);

    virtual ByteVector read(unsigned short code,
                            const std::vector<unsigned short> &blocks);

    /**
    * \brief Read data from the chip.
    * \param codes Service / Area codes list.
    * \param blocks Blocks list.
    * \return Data read.
    */
    virtual ByteVector read(const std::vector<unsigned short> &codes,
                            const std::vector<unsigned short> &blocks) = 0;

    virtual void write(unsigned short code, const std::vector<unsigned short> &blocks,
                       const ByteVector &data);

    virtual void write(unsigned short code, unsigned short block, const ByteVector &data);

    /**
    * \brief Write data to the chip.
    * \param codes Service / Area codes list.
    * \param blocks Blocks list.
    * \param data Data to write.
    */
    virtual void write(const std::vector<unsigned short> &codes,
                       const std::vector<unsigned short> &blocks,
                       const ByteVector &data) = 0;
};
}

#endif