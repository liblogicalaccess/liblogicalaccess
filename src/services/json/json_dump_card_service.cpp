#include <logicalaccess/myexception.hpp>
#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/stringdatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/binarydatafield.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/services/accesscontrol/formatinfos.hpp>
#include <logicalaccess/services/storage/storagecardservice.hpp>
#include <logicalaccess/services/accesscontrol/accesscontrolcardservice.hpp>
#include <logicalaccess/utils.hpp>
#include "logicalaccess/services/json/json_dump_card_service.hpp"
#include "nlohmann/json.hpp"

namespace logicalaccess
{

JsonDumpCardService::~JsonDumpCardService() {}

JsonDumpCardService::JsonDumpCardService(const std::shared_ptr<Chip> &chip)
    : CardService(chip, CardServiceType::CST_JSON_DUMP)
{
}

std::shared_ptr<Format> createFormat(const nlohmann::json &encodingFormat)
{
    const std::map<std::string, std::shared_ptr<Format>> formatType = {
        {"Wiegand-26", std::make_shared<Wiegand26Format>()},
        {"Wiegand-34", std::make_shared<Wiegand34Format>()},
        {"Wiegand-34-WithFacility", std::make_shared<Wiegand34WithFacilityFormat>()},
        {"Wiegand-37", std::make_shared<Wiegand37Format>()},
        {"Wiegand-37-WithFacility", std::make_shared<Wiegand37WithFacilityFormat>()}};

    // Get standard format
    if (formatType.count(encodingFormat.at("name").get<std::string>()))
        return formatType.at(encodingFormat.at("name").get<std::string>());

    // It is a custom format
    unsigned int offset = 0;
    auto customFormat   = std::make_shared<CustomFormat>();
    customFormat->setName(encodingFormat.at("name").get<std::string>());
    customFormat->setRepeatable(encodingFormat.at("is_repeated").get<bool>());
    std::vector<std::shared_ptr<DataField>> fields;
    for (const auto &field : encodingFormat.at("fields"))
    {
        std::shared_ptr<DataField> dataField;
        if (field.at("type").get<std::string>() == "FIELD_TYPE_STRING")
        {
            dataField = std::make_shared<StringDataField>();
            std::dynamic_pointer_cast<StringDataField>(dataField)->setDataLength(
                field.at("len").get<int>());
            std::dynamic_pointer_cast<StringDataField>(dataField)->setValue(
                field.at("default_value").get<std::string>());
            std::dynamic_pointer_cast<StringDataField>(dataField)->setIsIdentifier(
                field.at("is_identifier").get<bool>());
        }
        else if (field.at("type").get<std::string>() == "FIELD_TYPE_NUMBER")
        {
            dataField = std::make_shared<NumberDataField>();
            std::dynamic_pointer_cast<NumberDataField>(dataField)->setDataLength(
                field.at("len").get<int>());
            int numb;
            std::istringstream(field.at("default_value").get<std::string>()) >> numb;
            std::dynamic_pointer_cast<NumberDataField>(dataField)->setValue(numb);
            std::dynamic_pointer_cast<NumberDataField>(dataField)->setIsIdentifier(
                field.at("is_identifier").get<bool>());
        }
        else if (field.at("type").get<std::string>() == "FIELD_TYPE_BINARY")
        {
            dataField = std::make_shared<BinaryDataField>();
            auto data =
                BufferHelper::fromHexString(field.at("default_value").get<std::string>());
            std::dynamic_pointer_cast<BinaryDataField>(dataField)->setValue(data);
            std::dynamic_pointer_cast<BinaryDataField>(dataField)->setDataLength(
                field.at("len").get<int>());
            std::dynamic_pointer_cast<BinaryDataField>(dataField)->setIsIdentifier(
                field.at("is_identifier").get<bool>());
        }
        dataField->setName(field.at("name").get<std::string>());
        dataField->setPosition(offset);
        offset += field.at("len").get<int>();
        fields.push_back(dataField);
    }

    customFormat->setFieldList(fields);
    return customFormat;
}

void JsonDumpCardService::extract_formats(const nlohmann::json &json)
{
    try
    {
        for (const auto &format_element : json.at("formats").items())
        {
            auto fmt                       = createFormat(format_element.value());
            formats_[format_element.key()] = fmt;
        }
    }
    catch (const nlohmann::json::exception &e)
    {
        std::stringstream error;
        error << "JSON error when attempting to parse formats: " << e.what();
        throw LibLogicalAccessException(error.str());
    }
}

void JsonDumpCardService::extract_keys(const nlohmann::json &json)
{
    try
    {
        for (const auto &key_element : json.at("keyValues").items())
        {
            auto key                 = create_key(key_element.value());
            keys_[key_element.key()] = key;
        }
    }
    catch (const nlohmann::json::exception &e)
    {
        std::stringstream error;
        error << "JSON error when attempting to parse keys: " << e.what();
        throw LibLogicalAccessException(error.str());
    }
}

// for a single file / format read.
nlohmann::json result_to_json(const std::shared_ptr<logicalaccess::Format> &format)
{
    nlohmann::json json_result;
    for (const auto &field : format->getFieldList())
    {
        if (std::dynamic_pointer_cast<logicalaccess::NumberDataField>(field))
        {
            auto ndf = std::dynamic_pointer_cast<logicalaccess::NumberDataField>(field);
            json_result[field->getName()]["type"]         = "NUMBER";
            json_result[field->getName()]["value"]        = ndf->getValue();
            json_result[field->getName()]["isIdentifier"] = ndf->getIsIdentifier();
        }
        else if (std::dynamic_pointer_cast<logicalaccess::StringDataField>(field))
        {
            auto sdf = std::dynamic_pointer_cast<logicalaccess::StringDataField>(field);
            json_result[field->getName()]["type"] = "STRING";
            auto str_value                        = sdf->getValue();
            str_value.erase(std::remove(str_value.begin(), str_value.end(), 0),
                            str_value.end());
            json_result[field->getName()]["value"]        = str_value;
            json_result[field->getName()]["isIdentifier"] = sdf->getIsIdentifier();
        }
        else if (std::dynamic_pointer_cast<logicalaccess::BinaryDataField>(field))
        {
            auto bdf = std::dynamic_pointer_cast<logicalaccess::BinaryDataField>(field);
            json_result[field->getName()]["type"] = "BINARY";
            auto data                             = bdf->getValue();

            std::string dataString = base64_encode(data.data(), data.size());
            json_result[field->getName()]["value"]        = dataString;
            json_result[field->getName()]["isIdentifier"] = bdf->getIsIdentifier();
        }
    }

    return json_result;
}

std::string JsonDumpCardService::dump(const std::string &json_template)
{
    nlohmann::json json = nlohmann::json::parse(json_template);

    extract_formats(json);
    extract_keys(json);
    configure_format_infos(json);

    // Now we rely on AccessControl service to read each format.
    nlohmann::json json_result = {};
    auto acs                   = getChip()->getService<AccessControlCardService>();

    // Extract CSN.
    json_result["ADDITIONAL_DATA"]         = {};
    json_result["ADDITIONAL_DATA"]["CSN"]["type"] = "STRING";
    json_result["ADDITIONAL_DATA"]["CSN"]["value"] =
        logicalaccess::BufferHelper::getHex(getChip()->getChipIdentifier());

    for (const auto &format_info : format_infos_)
    {
        if (!format_info.second->getFormat()->isRepeatable())
        {
            auto result = acs->readFormat(format_info.second->getFormat(),
                                          format_info.second->getLocation(),
                                          format_info.second->getAiToUse());

            json_result[format_info.first] = result_to_json(result);
        }
        else
        {
            json_result[format_info.first] = nlohmann::json::array();
            while (true)
            {
                try
                {
                    auto result      = acs->readFormat(format_info.second->getFormat(),
                                                  format_info.second->getLocation(),
                                                  format_info.second->getAiToUse());
                    bool full_zeroes = true;
                    for (const auto &b : result->getLinearData())
                    {
                        if (b != 0x00)
                        {
                            full_zeroes = false;
                            break;
                        }
                    }
                    if (full_zeroes)
                    {
                        // We read only zeroes. We can assume that we read everything
                        // that was available.
                        break;
                    }

                    json_result[format_info.first].push_back(result_to_json(result));
                    // Now increment location, and try again.
                    assert(format_info.second->getFormat()->getDataLength() % 8 == 0);
                    format_info.second->getLocation()->incrementOffset(
                        format_info.second->getFormat()->getDataLength() / 8);
                }
                catch (const std::exception &e)
                {
                    // todo: handle this better.
                    // For now we assume failure means we are done reading.
                    break;
                }
            }
        }
    }
    return json_result.dump(4);
}
}
