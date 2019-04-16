#pragma once

#include <logicalaccess/services/json/json_dump_card_service.hpp>

namespace logicalaccess
{

class DESFireJsonDumpCardService : public JsonDumpCardService
{
  public:
    ~DESFireJsonDumpCardService();
    explicit DESFireJsonDumpCardService(const std::shared_ptr<Chip> &chip);

  private:
    std::shared_ptr<Key> create_key(const nlohmann::json &key_description) override;

    void configure_format_infos(const nlohmann::json &json) override;
};
}