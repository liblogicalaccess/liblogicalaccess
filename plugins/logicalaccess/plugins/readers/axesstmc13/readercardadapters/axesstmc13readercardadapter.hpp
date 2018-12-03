/**
 * \file axesstmc13readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default AxessTMC13 reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/readers/axesstmc13/lla_readers_axesstmc13_api.hpp>
#include <string>
#include <vector>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default AxessTMC13 reader/card adapter class.
 */
class LLA_READERS_AXESSTMC13_API AxessTMC13ReaderCardAdapter : public ReaderCardAdapter
{
  public:
    static const unsigned char START0; /**< \brief The first start byte. */
    static const unsigned char START1; /**< \brief The second start byte. */
    static const unsigned char CR;     /**< \brief The stop byte. */

    /**
     * \brief Constructor.
     */
    AxessTMC13ReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~AxessTMC13ReaderCardAdapter();

    /**
     * \brief Adapt the command to send to the reader.
     * \param command The command to send.
     * \return The adapted command to send.
     */
    ByteVector adaptCommand(const ByteVector &command) override;

    /**
     * \brief Adapt the asnwer received from the reader.
     * \param answer The answer received.
     * \return The adapted answer received.
     */
    ByteVector adaptAnswer(const ByteVector &answer) override;
};
}

#endif /* LOGICALACCESS_DEFAULTAXESSTMC13READERCARDADAPTER_H */