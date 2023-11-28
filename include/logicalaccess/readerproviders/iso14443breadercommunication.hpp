/**
 * \file iso14443breadercommunication.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief ISO14443-3B Reader communication.
 */

#ifndef LOGICALACCESS_ISO14443BREADERCOMMUNICATION_HPP
#define LOGICALACCESS_ISO14443BREADERCOMMUNICATION_HPP

#include <logicalaccess/readerproviders/readercommunication.hpp>

#include <string>
#include <vector>

#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A ISO14443-3B reader communication base class.
 */
class LLA_CORE_API ISO14443BReaderCommunication
{
  public:
    virtual ~ISO14443BReaderCommunication() = default;

    /**
 * \brief Send a REQB command from the PCD to the PICC.
 * \param afi The AFI value.
 * \return The ATQB PICC result.
 */
    virtual ByteVector requestB(unsigned char afi = 0x00) = 0;

    /**
     * \brief Send a HLTB command from the PCD to the PICC.
     */
    virtual void haltB() = 0;

    /**
     * \brief Send a attrib command from the PCD to the PICC.
     */
    virtual void attrib() = 0;

    /**
     * \brief Manage collision.
     * \param afi The AFI value.
     * \return The chip UID.
     */
    virtual ByteVector anticollisionB(unsigned char afi = 0x00) = 0;
};
}

#endif /* LOGICALACCESS_ISO14443BREADERCOMMUNICATION_HPP */