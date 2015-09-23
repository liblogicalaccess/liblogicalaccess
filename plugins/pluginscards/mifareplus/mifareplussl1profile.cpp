/**
 * \file mifareplussl1profile.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief MifarePlus card profiles.
 */

#include <logicalaccess/logs.hpp>
#include "mifareplussl1profile.hpp"
#include "mifareplusaccessinfo_sl1.hpp"

namespace logicalaccess
{
    std::shared_ptr<AccessInfo> MifarePlusSL1Profile::createAccessInfo() const
    {
        std::shared_ptr<MifarePlusSL1AccessInfo> ret;
        ret.reset(new MifarePlusSL1AccessInfo());
        return ret;
    }

    MifarePlusSL1Profile::MifarePlusSL1Profile() :
		aesAuthKey_(new AES128Key())
    {

    }
}