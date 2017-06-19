//
// Created by xaqq on 7/2/15.
//

#ifndef LIBLOGICALACCESS_MIFAREPLSSTORAGECARDSERVICE_SL1_H
#define LIBLOGICALACCESS_MIFAREPLSSTORAGECARDSERVICE_SL1_H


#include "../mifare/mifarestoragecardservice.hpp"

namespace logicalaccess
{
    class LIBLOGICALACCESS_API MifarePlusStorageCardServiceSL1 : public MifareStorageCardService
    {

    public:
        MifarePlusStorageCardServiceSL1(std::shared_ptr<Chip> chip);

        virtual void erase(std::shared_ptr<Location> location,
                           std::shared_ptr<AccessInfo> aiToUse) override;

        virtual void writeData(std::shared_ptr<Location> location,
                               std::shared_ptr<AccessInfo> aiToUse,
                               std::shared_ptr<AccessInfo> aiToWrite,
                               const std::vector<unsigned char> &data,
                               CardBehavior behaviorFlags) override;

        virtual std::vector<unsigned char> readData(std::shared_ptr<Location> location,
                                                    std::shared_ptr<AccessInfo> aiToUse,
                                                    size_t length,
                                                    CardBehavior behaviorFlags) override;

        virtual std::vector<unsigned char> readDataHeader(std::shared_ptr<Location> location,
														  std::shared_ptr<AccessInfo> aiToUse) override;

    private:
        void authenticate_if_needed(std::shared_ptr<AccessInfo>);

        /**
         * We perform AES authentication at most once, for performance
         * reason.
         */
        bool has_been_authenticated_;
    };
}

#endif //LIBLOGICALACCESS_MIFAREPLSSTORAGECARDSERVICE_SL1_H
