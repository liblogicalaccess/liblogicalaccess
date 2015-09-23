//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP
#define LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP

#include "../mifare/mifareaccessinfo.hpp"
#include "logicalaccess/cards/aes128key.hpp"
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
    class LIBLOGICALACCESS_API MifarePlusSL1AccessInfo : public MifareAccessInfo
    {
      public:
        MifarePlusSL1AccessInfo();

        virtual void serialize(boost::property_tree::ptree &parentNode) override;

        virtual void unSerialize(boost::property_tree::ptree &parentNode) override;

        virtual std::string getDefaultXmlNodeName() const;

        /**
         * Key for card authentication in SL1.
         */
        std::shared_ptr<AES128Key> aesAuthKey;
    };
}

#endif //LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP
