/**
 * \file felicastoragecardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief FeliCa storage card service.
 */

#include <logicalaccess/logs.hpp>
#include "felicastoragecardservice.hpp"
#include "felicalocation.hpp"
#include "logicalaccess/cards/locationnode.hpp"

namespace logicalaccess
{
	FeliCaStorageCardService::FeliCaStorageCardService(std::shared_ptr<Chip> chip)
        : StorageCardService(chip)
    {
    }

	FeliCaStorageCardService::~FeliCaStorageCardService()
    {
    }

	void FeliCaStorageCardService::erase()
    {
        std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
        std::vector<unsigned short> codes = cmd->getSystemCodes();
        std::vector<unsigned short> blocks;
        for (unsigned short i = 1; i < 14; ++i)
        {
            blocks.push_back(i);
        }
        std::vector<unsigned char> data(codes.size() * blocks.size() * 16, 0x00);
        cmd->write(codes, blocks, data);
    }

	void FeliCaStorageCardService::erase(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo> aiToUse)
    {
        std::shared_ptr<FeliCaLocation> flocation = std::dynamic_pointer_cast<FeliCaLocation>(location);
        std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();
        std::vector<unsigned short> blocks;
        for (unsigned short i = flocation->block; i < 14; ++i)
        {
            blocks.push_back(i);
        }
        std::vector<unsigned char> data(blocks.size() * 16, 0x00);
        cmd->write(flocation->code, blocks, data);
    }

    void FeliCaStorageCardService::writeData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo>, std::shared_ptr<AccessInfo>, const std::vector<unsigned char>& data, CardBehavior cardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<FeliCaLocation> icLocation = std::dynamic_pointer_cast<FeliCaLocation>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a FeliCaLocation.");

        std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();

        if ((cardBehavior & CB_AUTOSWITCHAREA) == CB_AUTOSWITCHAREA)
        {
            unsigned int i = 0;
            while (i < data.size())
            {
                cmd ->write(icLocation->code,
                    icLocation->block + (unsigned int)(i / 16),
                    std::vector<unsigned char>(data.cbegin() + i, (i + 16 < data.size()) ? data.begin() + i + 16 : data.cend())
                );
                i += 16;
            }
        }
        else
        {
            cmd->write(icLocation->code, icLocation->block, data);
        }
    }

	std::vector<unsigned char> FeliCaStorageCardService::readData(std::shared_ptr<Location> location, std::shared_ptr<AccessInfo>, size_t length, CardBehavior cardBehavior)
    {
        EXCEPTION_ASSERT_WITH_LOG(location, std::invalid_argument, "location cannot be null.");

		std::shared_ptr<FeliCaLocation> icLocation = std::dynamic_pointer_cast<FeliCaLocation>(location);

        EXCEPTION_ASSERT_WITH_LOG(icLocation, std::invalid_argument, "location must be a FeliCaLocation.");

        std::shared_ptr<FeliCaCommands> cmd = getFeliCaChip()->getFeliCaCommands();

        std::vector<unsigned char> data;
        if ((cardBehavior & CB_AUTOSWITCHAREA) == CB_AUTOSWITCHAREA)
        {
            unsigned int i = 0;
            while (i < length)
            {
                std::vector<unsigned char> bdata = cmd->read(icLocation->code, icLocation->block + (unsigned int)(i / 16));
                if (bdata.size() == 0)
                    break;
                data.insert(data.end(), bdata.begin(), bdata.end());
                i += bdata.size();
            }
        }
        else
        {
            data = cmd->read(icLocation->code, icLocation->block);
        }

        if (data.size() > length)
        {
            data = std::vector<unsigned char>(data.begin(), data.begin() + length);
        }

        return data;
    }

	unsigned int FeliCaStorageCardService::readDataHeader(std::shared_ptr<Location>, std::shared_ptr<AccessInfo>, void*, size_t)
    {
        return 0;
    }
}