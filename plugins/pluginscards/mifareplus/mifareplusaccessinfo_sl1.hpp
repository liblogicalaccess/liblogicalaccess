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
        MifarePlusSL1AccessInfo()
            : aesAuthKey(new AES128Key())
        {
        }

        virtual void serialize(boost::property_tree::ptree &parentNode) override
        {
            boost::property_tree::ptree node;
            MifareAccessInfo::serialize(node);

            boost::property_tree::ptree aes_key;
            aesAuthKey->serialize(aes_key);
            node.add_child("AESAuthKey", aes_key);

            parentNode.add_child(
                MifarePlusSL1AccessInfo::getDefaultXmlNodeName(), node);
        }

        virtual void
        unSerialize(boost::property_tree::ptree &parentNode) override
        {
            MifareAccessInfo::unSerialize(parentNode.get_child(
                MifareAccessInfo::getDefaultXmlNodeName()));
            aesAuthKey->unSerialize(parentNode.get_child("AESAuthKey"), "");
        }

        virtual std::string getDefaultXmlNodeName() const override
        {
            return "MifarePlusSL1AccessInfo";
        }

        /**
         * Key for card authentication in SL1.
         */
        std::shared_ptr<AES128Key> aesAuthKey;
    };
}

#endif //LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP
