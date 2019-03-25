/**
 * \file formatinfos.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Format information.
 */

#ifndef LOGICALACCESS_FORMATINFOS_HPP
#define LOGICALACCESS_FORMATINFOS_HPP

#include <logicalaccess/cards/chip.hpp>

namespace logicalaccess
{
/**
 * \brief Format information for a card type.
 */
class LLA_CORE_API FormatInfos
{
public:

    FormatInfos() = default;

    std::shared_ptr<Format> getFormat() const
    {
        return _format;
    }

    void setFormat(std::shared_ptr<Format> format)
    {
        _format = format;
    }

    std::shared_ptr<Location> getLocation() const
    {
        return _location;
    }

    void setLocation(std::shared_ptr<Location> location)
    {
        _location = location;
    }

    std::shared_ptr<AccessInfo> getAiToUse() const
    {
        return _aiToUse;
    }

    void setAiToUse(std::shared_ptr<AccessInfo> aitouse)
    {
        _aiToUse = aitouse;
    }

    std::shared_ptr<AccessInfo> getAiToWrite() const
    {
        return _aiToWrite;
    }

    void setAiToWrite(std::shared_ptr<AccessInfo> aittowrite)
    {
        _aiToWrite = aittowrite;
    }

private:
    std::shared_ptr<Format> _format;        /**< \brief Format information. */
    std::shared_ptr<Location> _location;    /**< \brief Location information. */
    std::shared_ptr<AccessInfo> _aiToUse;   /**< \brief Access information to use. */
    std::shared_ptr<AccessInfo> _aiToWrite; /**< \brief Access information to write. */
};
}

#endif /* LOGICALACCESS_FORMATINFOS_HPP */