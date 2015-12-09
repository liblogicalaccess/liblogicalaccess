#pragma once

#include <cstdint>
#include <cstddef>
#include <chrono>
#include "logicalaccess_api.hpp"
#include "lla_fwd.hpp"
#include <string>
#include <memory>
#include <map>

/**
 * Platform independent utils.
 */

namespace logicalaccess
{
uint32_t lla_htonl(uint32_t in);
uint16_t lla_htons(uint16_t in);

uint32_t lla_ntohl(uint32_t in);
uint16_t lla_ntohs(uint16_t in);

/**
 * This class provide a simple to get the elapsed time since
 * it's creation.
 *
 * The precision of the counter is milliseconds.
 */
class LIBLOGICALACCESS_API ElapsedTimeCounter
{
  public:
    ElapsedTimeCounter();

    /**
     * Returns the elapsed time since the creation of
     * the ElapsedTimeCounter object.
     */
    size_t elapsed() const;

  private:
    using TimePoint = std::chrono::steady_clock::time_point;

    TimePoint creation_;
};

class LIBLOGICALACCESS_API ReaderFactoryHelper
{
  public:
    using FactoryFunction =
        std::function<std::shared_ptr<ReaderUnit>(const std::string &)>;

    /**
     * Create a ReaderUnit object.
     *
     * The `identifier` is used to lookup the registered factory function to call,
     * while the `readerName` parameter is forwarded to the ReaderUnit constructor.
     */
    std::shared_ptr<ReaderUnit> instanciate(const std::string &identifier,
                                            const std::string &readerName);

    /**
     * Register a factory function for a reader unit whose identifier match
     * `identifier_regexp`.
     */
    void registerReader(const std::string &identifier_regexp, FactoryFunction fct);

    /**
     * Register a reader unit based on its USB vendor_id and product_id.
     *
     * Note that this will generate a string, "usb_identity::vendor:product"
     * and register this as the identifier.
     *
     * This call only make sense for some PCSC reader.
     */
    void registerReader(uint16_t vendor_id, uint16_t product_id,
                        FactoryFunction fct);

  private:
    using FactoryInfo = std::pair<std::string, FactoryFunction>;
    std::vector<FactoryInfo> factory_infos_;
};
}
