/**
 * \file pcscreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default PC/SC reader/card adapter.
 */

#ifndef LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP
#define LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP

#include <logicalaccess/plugins/cards/iso7816/readercardadapters/iso7816readercardadapter.hpp>
#include <logicalaccess/plugins/readers/pcsc/pcscreaderprovider.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default PC/SC reader/card adapter class.
 */
class LLA_READERS_PCSC_API PCSCReaderCardAdapter : public ISO7816ReaderCardAdapter
{
  public:
    /**
     *\ brief Constructor.
     */
    PCSCReaderCardAdapter();

    /**
     * \brief Destructor.
     */
    virtual ~PCSCReaderCardAdapter();
};
}

#endif /* LOGICALACCESS_DEFAULTPCSCREADERCARDADAPTER_HPP */