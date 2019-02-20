/**
 * \file iso7816response.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 response message.
 */

#ifndef LOGICALACCESS_ISO7816RESPONSE_HPP
#define LOGICALACCESS_ISO7816RESPONSE_HPP

#include <logicalaccess/plugins/cards/iso7816/lla_cards_iso7816_api.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <iomanip>

namespace logicalaccess
{
/**
 * \brief A ISO7816 response message.
 */
class LLA_CARDS_ISO7816_API ISO7816Response
{
  public:
    ISO7816Response() {}

    ISO7816Response(const ByteVector &data, unsigned char sw1, unsigned char sw2)
    {
        sw1_  = sw1;
        sw2_  = sw2;
        data_ = data;
    }

    explicit ISO7816Response(const ByteVector &data)
    {
        if (data.size() < 2)
        {
            THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                     "Missing SW1 SW2 Status Code.");
        }

        sw1_  = data.at(data.size() - 2);
        sw2_  = data.at(data.size() - 1);
        data_ = ByteVector(data.begin(), data.end() - 2);
    }

    unsigned char getSW1() const
    {
        return sw1_;
    }

    unsigned char getSW2() const
    {
        return sw2_;
    }

    const ByteVector &getData() const
    {
        return data_;
    }

    ByteVector getCompleteResponse() {
        auto resp = getData();
        resp.push_back(sw1_);
        resp.push_back(sw2_);
        return resp;
    }

  private:
    /**
     * \brief SW1 Response Status Code.
     */
    unsigned char sw1_;
    /**
     * \brief SW2 Response Status Code.
     */
    unsigned char sw2_;
    /**
     * \brief Data Response.
     */
    ByteVector data_;
};

inline LLA_CARDS_ISO7816_API std::ostream &operator<<(std::ostream &ss,
                                                      const ISO7816Response &response)
{
    std::stringstream tmp;
    tmp << std::hex;
    tmp << "ISO7816Response [";
    tmp << std::setfill('0') << std::setw(2) << +response.getSW1();
    tmp << std::setfill('0') << std::setw(2) << +response.getSW2() << "]:";
    tmp << std::resetiosflags(std::ios_base::basefield);
    tmp << response.getData();
    ss << tmp.str();
    return ss;
}
}

#endif /* LOGICALACCESS_ISO7816RESPONSE_HPP */
