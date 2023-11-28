/**
* \file felicaspringcardcommands.hpp
* \author Maxime C. <maxime@leosac.com>
* \brief FeliCa SpringCard card.
*/

#ifndef LOGICALACCESS_FELICASPRINGCARDCOMMANDS_HPP
#define LOGICALACCESS_FELICASPRINGCARDCOMMANDS_HPP

#include <logicalaccess/plugins/cards/felica/felicacommands.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
#define CMD_FELICASPRINGCARD "FeliCaSpringCard"

/**
* \brief The FeliCa card provider class for SpringCard reader.
*/
class LLA_READERS_PCSC_API FeliCaSpringCardCommands : public FeliCaCommands
{
  public:
    /**
    * \brief Constructor.
    */
    FeliCaSpringCardCommands();

    explicit FeliCaSpringCardCommands(std::string);

    /**
    * \brief Destructor.
    */
    virtual ~FeliCaSpringCardCommands();

    /**
    * \brief Get the PC/SC reader/card adapter.
    * \return The PC/SC reader/card adapter.
    */
    std::shared_ptr<PCSCReaderCardAdapter> getPCSCReaderCardAdapter() const
    {
        return std::dynamic_pointer_cast<PCSCReaderCardAdapter>(getReaderCardAdapter());
    }

    /**
    * \brief Get system codes.
    * \return System codes list.
    */
    std::vector<unsigned short> getSystemCodes() override;

    /**
    * \brief Request Services / Area versions.
    * \param codes Service / Area codes list.
    * \return Service / Area versions.
    */
    std::vector<unsigned short>
    requestServices(const std::vector<unsigned short> &codes) override;

    /**
    * \brief Request Response, current FeliCa mode.
    * \return FeliCa mode (0/1/2).
    */
    unsigned char requestResponse() override;

    /**
    * \brief Read data from the chip.
    * \param codes Service / Area codes list.
    * \param blocks Blocks list.
    * \return Data read.
    */
    ByteVector read(const std::vector<unsigned short> &codes,
                    const std::vector<unsigned short> &blocks) override;

    /**
    * \brief Write data to the chip.
    * \param codes Service / Area codes list.
    * \param blocks Blocks list.
    * \param data Data to write.
    */
    void write(const std::vector<unsigned short> &codes,
               const std::vector<unsigned short> &blocks,
               const ByteVector &data) override;
};
}

#endif /* LOGICALACCESS_FELICASPRINGCARDCOMMANDS_HPP */