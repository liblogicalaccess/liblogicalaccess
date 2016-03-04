#include "epass_identity_service.hpp"
#include "epass_access_info.hpp"
#include "epass_chip.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <logicalaccess/logs.hpp>

using namespace logicalaccess;

EPassIdentityService::EPassIdentityService(const std::shared_ptr<Chip> &chip)
    : IdentityCardService(chip)
{
}

std::string EPassIdentityService::getName()
{
    auto dg1 = getDG1();

    auto name = dg1.owner_name_;
    // We replace '<' by space, and truncate.

    boost::algorithm::replace_all(name, "<", " ");
    boost::algorithm::trim(name);
    return name;
}

ByteVector EPassIdentityService::getPicture()
{
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityService");

    auto cmd = chip->getEPassCommand();
    assert(cmd);

    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    auto dg2 = cmd->readDG2();

    if (dg2.infos_.size())
        return dg2.infos_[0].image_data_;
    return {};
}

std::shared_ptr<EPassChip> EPassIdentityService::getEPassChip()
{
    return std::dynamic_pointer_cast<EPassChip>(getChip());
}

std::shared_ptr<EPassAccessInfo> EPassIdentityService::getEPassAccessInfo()
{
    auto ai = std::dynamic_pointer_cast<EPassAccessInfo>(access_info_);
    EXCEPTION_ASSERT_WITH_LOG(
        ai, LibLogicalAccessException,
        "EPassIdentity Service expects a valid EPassAccessInfo");

    return ai;
}

EPassDG1 EPassIdentityService::getDG1()
{
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityService");

    auto cmd = chip->getEPassCommand();
    assert(cmd);

    if (dg1_cache_)
        return *dg1_cache_;
    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    dg1_cache_ = std::unique_ptr<EPassDG1>(new EPassDG1(cmd->readDG1()));

    return *dg1_cache_;
}

bool EPassIdentityService::get(MetaData what, ByteVector &out)
{
    bool ret = false;
    if (what == MetaData::PICTURE)
    {
        out = getPicture();
        ret = true;
    }
    return ret;
}

bool EPassIdentityService::get(MetaData what, std::string &out)
{
    bool ret = false;
    if (what == MetaData::NAME)
    {
        out = getName();
        ret = true;
    }
    else if (what == MetaData::NATIONALITY)
    {
        out = getDG1().nationality_;
        ret = true;
    }
    else if (what == MetaData::DOC_NO)
    {
        out = getDG1().doc_no_;
        ret = true;
    }
    return ret;
}

bool EPassIdentityService::get(MetaData what,
                               std::chrono::system_clock::time_point &out)
{
    bool ret = false;
    if (what == MetaData::BIRTHDATE)
    {
        out = getDG1().birthdate_;
        ret = true;
    }
    else if (what == MetaData::EXPIRATION)
    {
        out = getDG1().expiration_;
        ret = true;
    }
    return ret;
}
