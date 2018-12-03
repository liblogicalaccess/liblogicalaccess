/**
 * \file readercommunication.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Reader communication.
 */

#ifndef LOGICALACCESS_READERCOMMUNICATION_HPP
#define LOGICALACCESS_READERCOMMUNICATION_HPP

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <vector>

namespace logicalaccess
{
/**
 * \brief A reader communication base class.
 */
class LLA_CORE_API ReaderCommunication
{
  public:
    virtual ~ReaderCommunication() = default;

    ReaderCommunication()
    {
    }

    /**
     * \brief Send a request command from the PCD to the PICC.
     * \return The PICC answer to the request command.
     */
    virtual ByteVector request() = 0;

    /**
     * \brief Manage collision.
     * \return PICC serial number.
     */
    virtual ByteVector anticollision() = 0;

    /**
     * \brief Select a PICC.
     * \param uid The PICC serial number.
     */

    virtual void selectIso(const ByteVector &uid) = 0;
};
}

#endif /* LOGICALACCESS_READERCOMMUNICATION_HPP */