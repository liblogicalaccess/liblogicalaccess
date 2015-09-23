/**
 * \file mifareplussl1profile.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 card profiles.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSL1_HPP
#define LOGICALACCESS_MIFAREPLUSSL1_HPP

#include "mifareplusprofile.hpp"
#include "../mifare/mifareprofile.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <logicalaccess/cards/aes128key.hpp>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API MifarePlusSL1Profile : public MifareProfile
    {
    public:
        MifarePlusSL1Profile();

        void setAESAuthKey(std::shared_ptr<AES128Key> k)
        {
            aesAuthKey_ = k;
        }

        std::shared_ptr<AES128Key> getAESAuthKey()
        {
            return aesAuthKey_;
        }

        virtual std::shared_ptr<AccessInfo> createAccessInfo() const;


    protected:
        std::shared_ptr<AES128Key> aesAuthKey_;
    };
}

#endif /* LOGICALACCESS_MIFAREPLUS_HPP */