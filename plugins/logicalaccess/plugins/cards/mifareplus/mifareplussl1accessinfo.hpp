//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP
#define LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP

#include <logicalaccess/plugins/cards/mifare/mifareaccessinfo.hpp>
#include <logicalaccess/cards/aes128key.hpp>
#include <logicalaccess/plugins/cards/mifareplus/lla_cards_mifareplus_api.hpp>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
class LLA_CARDS_MIFAREPLUS_API MifarePlusSL1AccessInfo : public MifareAccessInfo
{
  public:
    MifarePlusSL1AccessInfo();

    void serialize(boost::property_tree::ptree &parentNode) override;

    void unSerialize(boost::property_tree::ptree &parentNode) override;

    std::string getDefaultXmlNodeName() const override;

    std::string getCardType() const override;

    /**
     * Key for card authentication in SL1.
     */
    std::shared_ptr<AES128Key> aesAuthKey;
};
}

#endif // LIBLOGICALACCESS_MIFAREPLUSACCESSINFO_SL1_HPP
