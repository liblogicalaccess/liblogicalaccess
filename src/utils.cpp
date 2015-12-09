#include "logicalaccess/utils.hpp"
#include <boost/asio.hpp>
#include <boost/regex.hpp>

using namespace std::chrono;

namespace logicalaccess
{
uint32_t lla_htonl(uint32_t in)
{
    return htonl(in);
}

uint16_t lla_htons(uint16_t in)
{
    return htons(in);
}

uint32_t lla_ntohl(uint32_t in)
{
    return ntohl(in);
}

uint16_t lla_ntohs(uint16_t in)
{
    return ntohs(in);
}

ElapsedTimeCounter::ElapsedTimeCounter()
{
    creation_ = steady_clock::now();
}

size_t ElapsedTimeCounter::elapsed() const
{
    auto diff = steady_clock::now() - creation_;
    return static_cast<size_t>(duration_cast<milliseconds>(diff).count());
}

std::shared_ptr<ReaderUnit>
ReaderFactoryHelper::instanciate(const std::string &identifier,
                                 const std::string &readerName)
{
    for (const auto &factory_info : factory_infos_)
    {
        const std::string &regxp = factory_info.first;
        auto call = factory_info.second;
        boost::regex r(regxp);

        if (boost::regex_match(identifier, r))
        {
            return call(readerName);
        }
    }
    return nullptr;
}

void ReaderFactoryHelper::registerReader(const std::string &identifier_regexp,
                                         ReaderFactoryHelper::FactoryFunction fct)
{
    factory_infos_.push_back(std::make_pair(identifier_regexp, fct));
}

void ReaderFactoryHelper::registerReader(uint16_t vendor_id, uint16_t product_id,
                                         ReaderFactoryHelper::FactoryFunction fct)
{
    std::stringstream ss;
    ss << "usb_identity::" << std::hex << vendor_id << "_" << product_id;

    registerReader(ss.str(), fct);
}
}
