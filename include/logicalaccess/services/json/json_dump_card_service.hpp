#pragma once

#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <nlohmann/json_fwd.hpp>
#include <logicalaccess/services/accesscontrol/formatinfos.hpp>

namespace logicalaccess {

#define JSON_DUMP_CARDSERVICE "JSONDump"

/**
 * A high level service that read data from a card and return
 * its content.
 *
 * The content to fetch, from format to keys, is described by JSON.
 * Similarly, the output is a JSON string representing the data
 * that have been extracted from the card.
 *
 * The input JSON format is RFID.onl format. Todo better documentation ?
 */
class LLA_CORE_API JsonDumpCardService : public CardService {
  public:
    constexpr static const CardServiceType service_type_ = CST_JSON_DUMP;

    virtual ~JsonDumpCardService();

    explicit JsonDumpCardService(const std::shared_ptr<Chip> &chip);

    std::string getCSType() override
    {
        return JSON_DUMP_CARDSERVICE;
    }

    /**
     * Configure the service, providing the input JSON that will be used
     * to create internal object required to properly dump the content of the card.
     *
     * It is required to call configure() before calling dump().
     */
    void configure(const std::string &json_template);

    /**
     * Dump the content of the content of the card, according the configuration
     * provided at the configure() call.
     */
    std::string dump();

    std::map<std::string, std::shared_ptr<Format>> formats_;
    std::map<std::string, std::shared_ptr<Key>> keys_;
    std::map<std::string, std::shared_ptr<FormatInfos>> format_infos_;

  private:
    void extract_formats(const nlohmann::json & json);

    /**
     * Create a properly typed key object that can be used against the given
     * card.
     */
    virtual std::shared_ptr<Key> create_key(const nlohmann::json &key_description) = 0;

    void extract_keys(const nlohmann::json & json);

    /**
     * Populate the format_infos_ map.
     * The subclass is expected to parse the provided JSON and create corresponding
     * FormatInfo with proper Format, Location and AccessInfo.
     */
    virtual void configure_format_infos(const nlohmann::json & json) = 0;

    bool configured_;
};

}
