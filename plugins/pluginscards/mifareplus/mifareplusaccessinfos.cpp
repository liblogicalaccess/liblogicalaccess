/**
 * \file MifarePlusAccessInfo.cpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Access informations for mifarePlus card.
 */

#include <boost/property_tree/ptree.hpp>
#include <iomanip>
#include "mifareplusaccessinfo.hpp"
#include "mifarepluschip.hpp"

namespace logicalaccess
{
    MifarePlusAccessInfo::MifarePlusAccessInfo(size_t sectorKeySize)
    {
        d_sectorKeySize = sectorKeySize;

        keyA = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
        keyA->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keyB = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
        keyB->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keyOriginality = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keyOriginality->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keyMastercard = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keyMastercard->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keyConfiguration = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keyConfiguration->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keySwitchL2 = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keySwitchL2->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keySwitchL3 = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keySwitchL3->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        keyAuthenticateSL1AES = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(MIFARE_PLUS_AES_KEY_SIZE));
        keyAuthenticateSL1AES->fromString(MIFARE_PLUS_DEFAULT_AESKEY);

        madKeyA = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
        madKeyA->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        madKeyB = std::shared_ptr<MifarePlusKey>(new MifarePlusKey(d_sectorKeySize));
        madKeyB->fromString(MIFARE_PLUS_DEFAULT_AESKEY);
        useMAD = false;
        sab.setTransportConfiguration();

        gpb = 0x00;
        madGPB = 0xC3; // DA: 1, MA: 1, RFU, ADV: 01
    }

    MifarePlusAccessInfo::~MifarePlusAccessInfo()
    {
    }

    std::string MifarePlusAccessInfo::getCardType() const
    {
        return CHIP_MIFAREPLUS4K;
    }

    void MifarePlusAccessInfo::generateInfos()
    {
        keyA->fromString(generateSimpleKey(keyA->getLength()));
        keyB->fromString(generateSimpleKey(keyB->getLength()));
        keyOriginality->fromString(generateSimpleKey(keyOriginality->getLength()));
        keyMastercard->fromString(generateSimpleKey(keyMastercard->getLength()));
        keyConfiguration->fromString(generateSimpleKey(keyConfiguration->getLength()));
        keySwitchL2->fromString(generateSimpleKey(keySwitchL2->getLength()));
        keySwitchL3->fromString(generateSimpleKey(keySwitchL3->getLength()));
        keyAuthenticateSL1AES->fromString(generateSimpleKey(keyAuthenticateSL1AES->getLength()));

        sab.setAReadBWriteConfiguration();
    }

    void MifarePlusAccessInfo::serialize(boost::property_tree::ptree& parentNode)
    {
        unsigned char buf[3];
        boost::property_tree::ptree node;
        sab.toArray(buf, sizeof(buf));
        std::ostringstream oss;
        oss << std::setfill('0');
        for (size_t i = 0; i < sizeof(buf); ++i)
        {
            oss << std::setw(2) << std::hex << static_cast<unsigned int>(buf[i]);
            if (i < sizeof(buf) - 1)
            {
                oss << " ";
            }
        }

        boost::property_tree::ptree ka;
        keyA->serialize(ka);
        node.add_child("KeyA", ka);
        boost::property_tree::ptree kb;
        keyB->serialize(kb);
        node.add_child("KeyB", kb);
        boost::property_tree::ptree ko;
        keyOriginality->serialize(ko);
        node.add_child("KeyOriginality", ko);
        boost::property_tree::ptree km;
        keyMastercard->serialize(km);
        node.add_child("KeyMastercard", km);
        boost::property_tree::ptree kc;
        keyConfiguration->serialize(kc);
        node.add_child("KeyConfiguration", kc);
        boost::property_tree::ptree ks2;
        keySwitchL2->serialize(ks2);
        node.add_child("KeySwitchL2", ks2);
        boost::property_tree::ptree ks3;
        keySwitchL3->serialize(ks3);
        node.add_child("KeySwitchL3", ks3);
        boost::property_tree::ptree kaa;
        keyAuthenticateSL1AES->serialize(kaa);
        node.add_child("KeyAuthenticateSL1AES", kaa);

        node.put("SectorAccessBits", oss.str());

        boost::property_tree::ptree madnode;
        madnode.put("Use", useMAD);
        boost::property_tree::ptree mada;
        madKeyA->serialize(mada);
        madnode.add_child("MADKeyA", mada);
        boost::property_tree::ptree madb;
        madKeyB->serialize(madb);
        madnode.add_child("MADKeyB", madb);
        node.add_child("MAD", madnode);

        parentNode.add_child(getDefaultXmlNodeName(), node);
    }

    void MifarePlusAccessInfo::unSerialize(boost::property_tree::ptree& node)
    {
        keyA->unSerialize(node.get_child("KeyA"), "");
        keyB->unSerialize(node.get_child("KeyB"), "");
        keyOriginality->unSerialize(node.get_child("KeyOriginality"), "");
        keyMastercard->unSerialize(node.get_child("KeyMastercard"), "");
        keyConfiguration->unSerialize(node.get_child("KeyConfiguration"), "");
        keySwitchL2->unSerialize(node.get_child("KeySwitchL2"), "");
        keySwitchL3->unSerialize(node.get_child("KeySwitchL3"), "");
        keyAuthenticateSL1AES->unSerialize(node.get_child("KeyAuthenticateSL1AES"), "");

        string sabstr = node.get_child("SectorAccessBits").get_value<std::string>();
        unsigned char buf[3];
        memset(buf, 0x00, sizeof(buf));
        std::istringstream iss(sabstr);
        for (size_t i = 0; i < sizeof(buf); ++i)
        {
            unsigned int tmp;
            if (!iss.good())
            {
                return;
            }
            iss >> std::hex >> tmp;

            buf[i] = static_cast<unsigned char>(tmp);
        }
        sab.fromArray(buf, sizeof(buf));

        boost::property_tree::ptree modnode = node.get_child("MAD");
        if (!modnode.empty())
        {
            useMAD = modnode.get_child("Use").get_value<bool>();
            madKeyA->unSerialize(modnode.get_child("MADKeyA"), "");
            madKeyB->unSerialize(modnode.get_child("MADKeyB"), "");
        }
    }

    std::string MifarePlusAccessInfo::getDefaultXmlNodeName() const
    {
        return "MifarePlusAccessInfo";
    }

    bool MifarePlusAccessInfo::operator==(const AccessInfo& ai) const
    {
        if (!AccessInfo::operator==(ai))
            return false;

        if (typeid(ai) != typeid(MifarePlusAccessInfo))
            return false;

        const MifarePlusAccessInfo* mAi = dynamic_cast<const MifarePlusAccessInfo*>(&ai);

        if (!mAi)
            return false;

        return (keyA == mAi->keyA
            && keyB == mAi->keyB
            && keyOriginality == mAi->keyOriginality
            && keyMastercard == mAi->keyMastercard
            && keyConfiguration == mAi->keyConfiguration
            && keySwitchL2 == mAi->keySwitchL2
            && keySwitchL3 == mAi->keySwitchL3
            && keyAuthenticateSL1AES == mAi->keyAuthenticateSL1AES
            && madKeyA == mAi->madKeyA
            && madKeyB == mAi->madKeyB
            && useMAD == mAi->useMAD
            );
    }

    size_t MifarePlusAccessInfo::SectorAccessBits::toArray(void* buf, size_t buflen) const
    {
        if (buflen < 3)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
        }

        memset(buf, 0x00, 3);

        unsigned char* data = reinterpret_cast<unsigned char*>(buf);

        data[1] = (d_sector_trailer_access_bits.c1 << 7) | (d_data_blocks_access_bits[2].c1 << 6) | (d_data_blocks_access_bits[1].c1 << 5) | (d_data_blocks_access_bits[0].c1 << 4);
        data[2] = (d_sector_trailer_access_bits.c3 << 7) | (d_data_blocks_access_bits[2].c3 << 6) | (d_data_blocks_access_bits[1].c3 << 5) | (d_data_blocks_access_bits[0].c3 << 4);
        data[2] |= (d_sector_trailer_access_bits.c2 << 3) | (d_data_blocks_access_bits[2].c2 << 2) | (d_data_blocks_access_bits[1].c2 << 1) | (d_data_blocks_access_bits[0].c2 << 0);
        data[0] = ((~data[2]) << 4) & 0xF0;
        data[0] |= ((~data[1]) >> 4) & 0x0F;
        data[1] |= ((~data[2]) >> 4) & 0x0F;

        return 3;
    }

    bool MifarePlusAccessInfo::SectorAccessBits::fromArray(const void* buf, size_t buflen)
    {
        if (buflen < 3)
        {
            THROW_EXCEPTION_WITH_LOG(std::invalid_argument, "The buffer is too short.");
        }

        SectorAccessBits sab;

        const unsigned char* data = reinterpret_cast<const unsigned char*>(buf);

        sab.d_sector_trailer_access_bits.c1 = ((data[1] & 0x80) != 0);
        sab.d_sector_trailer_access_bits.c2 = ((data[2] & 0x08) != 0);
        sab.d_sector_trailer_access_bits.c3 = ((data[2] & 0x80) != 0);
        sab.d_data_blocks_access_bits[2].c1 = ((data[1] & 0x40) != 0);
        sab.d_data_blocks_access_bits[2].c2 = ((data[2] & 0x04) != 0);
        sab.d_data_blocks_access_bits[2].c3 = ((data[2] & 0x40) != 0);
        sab.d_data_blocks_access_bits[1].c1 = ((data[1] & 0x20) != 0);
        sab.d_data_blocks_access_bits[1].c2 = ((data[2] & 0x02) != 0);
        sab.d_data_blocks_access_bits[1].c3 = ((data[2] & 0x20) != 0);
        sab.d_data_blocks_access_bits[0].c1 = ((data[1] & 0x10) != 0);
        sab.d_data_blocks_access_bits[0].c2 = ((data[2] & 0x01) != 0);
        sab.d_data_blocks_access_bits[0].c3 = ((data[2] & 0x10) != 0);

        unsigned char test[3];

        if (sab.toArray(test, 3) > 0)
        {
            if (memcmp(test, buf, 3) == 0)
            {
                operator=(sab);

                return true;
            }
        }

        return false;
    }

    void MifarePlusAccessInfo::SectorAccessBits::setTransportConfiguration()
    {
        d_sector_trailer_access_bits.c1 = false;
        d_sector_trailer_access_bits.c2 = false;
        d_sector_trailer_access_bits.c3 = true;

        for (unsigned int i = 0; i < 3; ++i)
        {
            d_data_blocks_access_bits[i].c1 = false;
            d_data_blocks_access_bits[i].c2 = false;
            d_data_blocks_access_bits[i].c3 = false;
        }
    }

    void MifarePlusAccessInfo::SectorAccessBits::setAReadBWriteConfiguration()
    {
        d_sector_trailer_access_bits.c1 = false;
        d_sector_trailer_access_bits.c2 = true;
        d_sector_trailer_access_bits.c3 = true;

        for (unsigned int i = 0; i < 3; ++i)
        {
            d_data_blocks_access_bits[i].c1 = true;
            d_data_blocks_access_bits[i].c2 = false;
            d_data_blocks_access_bits[i].c3 = false;
        }
    }
}