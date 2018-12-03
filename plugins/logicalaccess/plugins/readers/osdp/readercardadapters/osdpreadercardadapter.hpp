/**
 * \file osdpreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader/card adapter.
 */

#ifndef LOGICALACCESS_OSDPREADERCARDADAPTER_HPP
#define LOGICALACCESS_OSDPREADERCARDADAPTER_HPP

#include <logicalaccess/readerproviders/datatransport.hpp>
#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/iso7816/commands/desfireiso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/osdp/osdpcommands.hpp>

namespace logicalaccess
{
/**
 * \brief A reader/card adapter for Transparent Smart Card Interface OSDP.
 */
class LLA_READERS_OSDP_API OSDPReaderCardAdapter : public ISO7816ReaderCardAdapter
{
  public:
    OSDPReaderCardAdapter(std::shared_ptr<OSDPCommands> command, unsigned char address,
                          std::shared_ptr<DESFireISO7816ResultChecker> resultChecker);

    ~OSDPReaderCardAdapter();

    ByteVector sendCommand(const ByteVector &command, long timeout = -1) override;

  protected:
    std::shared_ptr<DataTransport> d_dataTransport;

    std::shared_ptr<OSDPCommands> m_commands;

    std::shared_ptr<DESFireISO7816ResultChecker> d_resultChecker;

    unsigned char m_address;
};
}

#endif /* LOGICALACCESS_OSDPREADERCARDADAPTER_HPP */
