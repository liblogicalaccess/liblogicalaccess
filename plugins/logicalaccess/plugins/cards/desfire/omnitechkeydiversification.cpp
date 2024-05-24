#include <logicalaccess/plugins/cards/desfire/desfirekey.hpp>
#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/desfire/omnitechkeydiversification.hpp>
#include <memory>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/cards/desfire/desfirecrypto.hpp>
#include <vector>
#include <logicalaccess/myexception.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>

namespace logicalaccess
{
void OmnitechKeyDiversification::initDiversification(ByteVector identifier,
                                                     unsigned int /*AID*/,
                                                     std::shared_ptr<Key> /*key*/,
                                                     unsigned char /*keyno*/,
                                                     ByteVector &diversify)
{
    if (identifier.size() > 0)
    {
        diversify.resize(2 * identifier.size() + 2);
        diversify[0] = 0xFF;
        memcpy(&diversify[1], &identifier[0], identifier.size());
        diversify[8] = 0x00;
        memcpy(&diversify[9], &identifier[0], identifier.size());
        for (unsigned char i = 0; i < 7; ++i)
        {
            diversify[9 + i] ^= 0xFF;
        }
    }
    else
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Chip identifier is required for key diversification.");
    }
}

ByteVector OmnitechKeyDiversification::getDiversifiedKey(std::shared_ptr<Key> key,
                                                         ByteVector diversify)
{
    LOG(LogLevel::INFOS) << "Using key diversification Omnitech with div : "
                         << BufferHelper::getHex(diversify);
    ByteVector keydiv;

    std::shared_ptr<DESFireKey> desfirekey = std::dynamic_pointer_cast<DESFireKey>(key);

    ByteVector iv;
    // Two time, to have ECB and not CBC mode
    ByteVector vkeydata;
    if (desfirekey->isEmpty())
    {
        vkeydata.resize(desfirekey->getLength(), 0x00);
    }
    else
    {
        ByteVector keycipher = desfirekey->getData();
        vkeydata.insert(vkeydata.end(), keycipher.begin(), keycipher.end());
    }

    ByteVector r = DESFireCrypto::sam_CBC_send(
        vkeydata, iv, ByteVector(diversify.begin(), diversify.begin() + 8));
    ByteVector r2 = DESFireCrypto::sam_CBC_send(
        vkeydata, iv, ByteVector(diversify.begin() + 8, diversify.begin() + 16));

    for (unsigned char i = 0; i < 8; ++i)
    {
        r[7 - i] = static_cast<unsigned char>(
            (r[7 - i] & 0xFE) | ((desfirekey->getKeyVersion() >> i) & 0x01));
        r2[i] = static_cast<unsigned char>(r2[i] & 0xFE);
    }

    keydiv.insert(keydiv.end(), r.begin(), r.end());
    keydiv.insert(keydiv.end(), r2.begin(), r2.end());
    return keydiv;
}

void OmnitechKeyDiversification::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;
    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void OmnitechKeyDiversification::unSerialize(boost::property_tree::ptree & /*node*/)
{
}
}