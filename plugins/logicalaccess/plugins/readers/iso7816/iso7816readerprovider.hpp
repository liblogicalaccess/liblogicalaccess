/**
* \file iso7816readerprovider.hpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ISO7816 card reader provider.
*/

#ifndef LOGICALACCESS_READERISO7816_PROVIDER_HPP
#define LOGICALACCESS_READERISO7816_PROVIDER_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/plugins/readers/iso7816/iso7816readerunit.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief ISO7816 Reader Provider base class.
 */
class LLA_READERS_ISO7816_API ISO7816ReaderProvider : public ReaderProvider
{
  public:
    using ReaderProvider::createReaderUnit;

    /**
     * \brief Create a new reader unit for the reader provider.
     * \return A reader unit.
     */
    virtual std::shared_ptr<ISO7816ReaderUnit>
    createReaderUnit(std::string readerunitname) = 0;
};
}

#endif /* LOGICALACCESS_READERISO7816_PROVIDER_HPP */