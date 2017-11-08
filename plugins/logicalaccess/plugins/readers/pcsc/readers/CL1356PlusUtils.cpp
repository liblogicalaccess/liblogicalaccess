#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include <cassert>
#include <iostream>
#include <string.h>
#include <logicalaccess/plugins/readers/pcsc/readers/CL1356PlusUtils.hpp>

using namespace logicalaccess;

CL1356PlusUtils::CardCount CL1356PlusUtils::parse_list_card(const ByteVector &in)
{
    // The response shall contains 8 bytes.
    EXCEPTION_ASSERT_WITH_LOG(in.size() == 6, LibLogicalAccessException,
                              "Response is not 8 bytes long.");

    CardCount ret;
    memset(&ret, 0x00, sizeof(CardCount));

    ret.detected_ = in[0];
    ret.present_  = in[1];
    ret.active_   = in[2];

    return ret;
}

CL1356PlusUtils::Info CL1356PlusUtils::parse_get_card_info(const ByteVector &in)
{
    auto itr                = in.begin();
    auto check_enough_bytes = [&](int n) {
        EXCEPTION_ASSERT_WITH_LOG(std::distance(itr, in.end()) >= n,
                                  LibLogicalAccessException,
                                  "Not enough bytes in response.");
    };

    Info info;
    memset(&info, 0x00, sizeof(info));
    if (*itr != 0x01)
    {
        std::cout << "itr = " << (*itr) << std::endl;
        assert(0);
    }
    ++itr; // skip card type

    check_enough_bytes(1);
    info.state_ = static_cast<Info::State>(*itr);
    ++itr;

    check_enough_bytes(2);
    itr += 2; // skip bit rate infos

    check_enough_bytes(1);
    info.card_identifier_ = *itr;
    ++itr;

    check_enough_bytes(1);
    auto len = *itr;
    ++itr;

    check_enough_bytes(len);
    for (int i = 0; i < len; ++i)
    {
        info.uid_.push_back(*itr);
        ++itr;
    }

    // ignore rest for now.
    return info;
}
