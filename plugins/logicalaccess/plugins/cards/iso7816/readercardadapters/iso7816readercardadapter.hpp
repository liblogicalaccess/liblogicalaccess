/**
 * \file iso7816readercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Default ISO7816 reader/card adapter.
 */

#ifndef LOGICALACCESS_ISO7816READERCARDADAPTER_HPP
#define LOGICALACCESS_ISO7816READERCARDADAPTER_HPP

#include <logicalaccess/cards/readercardadapter.hpp>
#include <logicalaccess/plugins/cards/iso7816/lla_cards_iso7816_api.hpp>
#include <string>
#include <vector>
#include <logicalaccess/plugins/llacommon/logs.hpp>

namespace logicalaccess
{
/**
 * \brief A default ISO7816 reader/card adapter class.
 */
class LLA_CARDS_ISO7816_API ISO7816ReaderCardAdapter : public ReaderCardAdapter
{
  public:
    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const ByteVector &data,
                                       unsigned char le);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, const ByteVector &data);

    /**
     * \brief Send an APDU command to the reader.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       const ByteVector &data);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char lc, unsigned char le);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2,
                                       unsigned char le);

    /**
     * \brief Send an APDU command to the reader without data.
     */
    virtual ByteVector sendAPDUCommand(unsigned char cla, unsigned char ins,
                                       unsigned char p1, unsigned char p2);
};
}

#endif /* LOGICALACCESS_ISO7816READERCARDADAPTER_HPP */