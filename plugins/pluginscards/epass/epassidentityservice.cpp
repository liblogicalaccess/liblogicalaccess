#include "epassidentityservice.hpp"
#include "epassaccessinfo.hpp"
#include "epasschip.hpp"
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
    LLA_LOG_CTX("EPassIdentityService::getPicture");
    auto chip = getEPassChip();
    EXCEPTION_ASSERT_WITH_LOG(chip, LibLogicalAccessException,
                              "No or invalid chip object in EPassIdentityService");
	
    auto cmd = chip->getEPassCommands();
    assert(cmd);

    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    auto dg2 = cmd->readDG2();

    if (dg2.infos_.size())
        return dg2.infos_[0].image_data_;
    return {};
}

std::shared_ptr<EPassChip> EPassIdentityService::getEPassChip() const
{
    return std::dynamic_pointer_cast<EPassChip>(getChip());
}

std::shared_ptr<EPassAccessInfo> EPassIdentityService::getEPassAccessInfo() const
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

    auto cmd = chip->getEPassCommands();
    assert(cmd);

    if (dg1_cache_)
        return *dg1_cache_;
    cmd->selectIssuerApplication();
    cmd->authenticate(getEPassAccessInfo()->mrz_);
    dg1_cache_ = std::make_unique<EPassDG1>(cmd->readDG1());

    return *dg1_cache_;
}

ByteVector EPassIdentityService::getData(MetaData what)
{
	ByteVector out;
    if (what == MetaData::PICTURE)
    {
        out = getPicture();
    }

	return out;
}

std::string EPassIdentityService::getString(MetaData what)
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

std::chrono::system_clock::time_point EPassIdentityService::getTime(MetaData what)
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
