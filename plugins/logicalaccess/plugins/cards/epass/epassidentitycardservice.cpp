#include <logicalaccess/plugins/cards/epass/epassidentitycardservice.hpp>
#include <logicalaccess/plugins/cards/epass/epassaccessinfo.hpp>
#include <logicalaccess/plugins/cards/epass/epasschip.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <memory>
#include <logicalaccess/plugins/llacommon/logs.hpp>

using namespace logicalaccess;

EPassIdentityCardService::EPassIdentityCardService(const std::shared_ptr<Chip> &chip)
    : IdentityCardService(chip)
{
}

std::string EPassIdentityCardService::getName()
{
    auto dg1 = getDG1();

    auto name = dg1.owner_name_;
    // We replace '<' by space, and truncate.

    boost::algorithm::replace_all(name, "<", " ");
    boost::algorithm::trim(name);
    return name;
}

ByteVector EPassIdentityCardService::getPicture()
{
    LLA_LOG_CTX("EPassIdentityCardService::getPicture");
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityCardService");

    auto cmd = chip->getEPassCommands();
    assert(cmd);

    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    auto dg2 = cmd->readDG2();

    if (dg2.infos_.size())
        return dg2.infos_[0].image_data_;
    return {};
}

std::shared_ptr<EPassChip> EPassIdentityCardService::getEPassChip() const
{
    return std::dynamic_pointer_cast<EPassChip>(getChip());
}

std::shared_ptr<EPassAccessInfo> EPassIdentityCardService::getEPassAccessInfo() const
{
    auto ai = std::dynamic_pointer_cast<EPassAccessInfo>(access_info_);
    EXCEPTION_ASSERT_WITH_LOG(ai, LibLogicalAccessException,
                              "EPassIdentity Service expects a valid EPassAccessInfo");

    return ai;
}

EPassDG1 EPassIdentityCardService::getDG1()
{
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityCardService");

    auto cmd = chip->getEPassCommands();
    assert(cmd);

    if (dg1_cache_)
        return *dg1_cache_;
    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    dg1_cache_ = std::make_unique<EPassDG1>(cmd->readDG1());

    return *dg1_cache_;
}

ByteVector EPassIdentityCardService::getData(MetaData what)
{
    ByteVector out;
    if (what == MetaData::PICTURE)
    {
        out = getPicture();
    }

    return out;
}

std::string EPassIdentityCardService::getString(MetaData what)
{
    std::string out;
    if (what == MetaData::NAME)
    {
        out = getName();
    }
    else if (what == MetaData::NATIONALITY)
    {
        out = getDG1().nationality_;
    }
    else if (what == MetaData::DOC_NO)
    {
        out = getDG1().doc_no_;
    }
    return out;
}

std::chrono::system_clock::time_point EPassIdentityCardService::getTime(MetaData what)
{
    std::chrono::system_clock::time_point out;
    if (what == MetaData::BIRTHDATE)
    {
        out = getDG1().birthdate_;
    }
    else if (what == MetaData::EXPIRATION)
    {
        out = getDG1().expiration_;
    }
    return out;
}
