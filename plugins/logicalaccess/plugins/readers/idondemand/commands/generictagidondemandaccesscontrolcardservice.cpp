/**
 * \file generictagidondemandaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag IdOnDemand Access Control Card service.
 */

#include <logicalaccess/plugins/readers/idondemand/commands/generictagidondemandaccesscontrolcardservice.hpp>
#include <logicalaccess/plugins/readers/idondemand/commands/generictagidondemandcommands.hpp>
#include <logicalaccess/plugins/cards/generictag/generictagchip.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>

#include <logicalaccess/plugins/readers/idondemand/readercardadapters/idondemandreadercardadapter.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand26format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand35format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37format.hpp>
#include <logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp>
#include <logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp>

#include <logicalaccess/services/accesscontrol/formats/format.hpp>

namespace logicalaccess
{
GenericTagIdOnDemandAccessControlCardService::
    GenericTagIdOnDemandAccessControlCardService(std::shared_ptr<Chip> chip)
    : GenericTagAccessControlCardService(chip)
{
}

GenericTagIdOnDemandAccessControlCardService::
    ~GenericTagIdOnDemandAccessControlCardService()
{
}

std::shared_ptr<Format> GenericTagIdOnDemandAccessControlCardService::readFormat(
    std::shared_ptr<Format> /*format*/, std::shared_ptr<Location> /*location*/,
    std::shared_ptr<AccessInfo> /*aiToUse*/)
{
    // Not implemented.
    return std::shared_ptr<Format>();
}

bool GenericTagIdOnDemandAccessControlCardService::writeFormat(
    std::shared_ptr<Format> format, std::shared_ptr<Location> /*location*/,
    std::shared_ptr<AccessInfo> /*aiToUse*/, std::shared_ptr<AccessInfo> /*aiToWrite*/)
{
    EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument,
                              "format to write can't be null.");

    // idOnDemand reader only support specific implemented formats.

    char tmp[64];
    std::string cmdstr = "";

    switch (format->getType())
    {
    case FT_WIEGAND26:
    {
        std::shared_ptr<Wiegand26Format> wf =
            std::dynamic_pointer_cast<Wiegand26Format>(format);
#if defined(__unix__)
        sprintf(tmp, "Wiegand 26 %d %llud", wf->getFacilityCode(), wf->getUid());
#else
        sprintf(tmp, "Wiegand 26 %d %llud", wf->getFacilityCode(), wf->getUid());
#endif
        cmdstr = std::string(tmp);
        break;
    }
    case FT_WIEGAND34FACILITY:
    {
        std::shared_ptr<Wiegand34WithFacilityFormat> wf =
            std::dynamic_pointer_cast<Wiegand34WithFacilityFormat>(format);
#if defined(__unix__)
        sprintf(tmp, "Wiegand 34 %d %llud", wf->getFacilityCode(), wf->getUid());
#else
        sprintf(tmp, "Wiegand 34 %d %llud 0", wf->getFacilityCode(), wf->getUid());
#endif
        cmdstr = std::string(tmp);
        break;
    }
    case FT_WIEGAND35:
    {
        std::shared_ptr<Wiegand35Format> wf =
            std::dynamic_pointer_cast<Wiegand35Format>(format);
#if defined(__unix__)
        sprintf(tmp, "Wiegand 35 %d %llud 3", wf->getCompanyCode(), wf->getUid());
#else
        sprintf(tmp, "Wiegand 35 %d %llud 3", wf->getCompanyCode(), wf->getUid());
#endif
        cmdstr = std::string(tmp);
        break;
    }
    case FT_WIEGAND37FACILITY:
    {
        std::shared_ptr<Wiegand37WithFacilityFormat> wf =
            std::dynamic_pointer_cast<Wiegand37WithFacilityFormat>(format);
#if defined(__unix__)
        sprintf(tmp, "Wiegand 37 %d %llud 2", wf->getFacilityCode(), wf->getUid());
#else
        sprintf(tmp, "Wiegand 37 %d %llud 2", wf->getFacilityCode(), wf->getUid());
#endif
        cmdstr = std::string(tmp);
        break;
    }
    case FT_WIEGAND37:
    {
        std::shared_ptr<Wiegand37Format> wf =
            std::dynamic_pointer_cast<Wiegand37Format>(format);
#if defined(__unix__)
        sprintf(tmp, "Wiegand 37 0 %lld 1", wf->getUid());
#else
        sprintf(tmp, "Wiegand 37 0 %lld 1", wf->getUid());
#endif
        cmdstr = std::string(tmp);
        break;
    }
    case FT_CUSTOM:
    {
        std::shared_ptr<CustomFormat> wf =
            std::dynamic_pointer_cast<CustomFormat>(format);
        std::shared_ptr<NumberDataField> fieldUid =
            std::dynamic_pointer_cast<NumberDataField>(wf->getFieldFromName("Uid"));
        std::shared_ptr<NumberDataField> fieldFacility =
            std::dynamic_pointer_cast<NumberDataField>(
                wf->getFieldFromName("Facility Code"));

        if (format->getName() == "Wiegand 32")
        {
#if defined(__unix__)
            sprintf(tmp, "Wiegand 32 %lld %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#else
            sprintf(tmp, "Wiegand 32 %lld %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#endif
            cmdstr = std::string(tmp);
        }
        else if (format->getName() == "Wiegand 37 With 13-bit Facility")
        {
#if defined(__unix__)
            sprintf(tmp, "Wiegand 37 %lld %llud 3", fieldFacility->getValue(),
                    fieldUid->getValue());
#else
            sprintf(tmp, "Wiegand 37 %lld %llud 3", fieldFacility->getValue(),
                    fieldUid->getValue());
#endif
            cmdstr = std::string(tmp);
        }
        else if (format->getName() == "Wiegand 40")
        {
#if defined(__unix__)
            sprintf(tmp, "Wiegand 40 %lld %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#else
            sprintf(tmp, "Wiegand 40 %lld %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#endif
            cmdstr = std::string(tmp);
        }
        else if (format->getName() == "Wiegand 42 (HID10313C compatible)")
        {
#if defined(__unix__)
            sprintf(tmp, "Wiegand 42 %llud %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#else
            sprintf(tmp, "Wiegand 42 %lld %llud", fieldFacility->getValue(),
                    fieldUid->getValue());
#endif
            cmdstr = std::string(tmp);
        }
        else if (format->getName() == "Honeywell")
        {
#if defined(__unix__)
            sprintf(tmp, "Honeywell %llud", fieldUid->getValue());
#else
            sprintf(tmp, "Honeywell %llud", fieldUid->getValue());
#endif
            cmdstr = std::string(tmp);
        }

        break;
    }
    case FT_UNKNOWN: break;
    case FT_WIEGAND34: break;
    case FT_WIEGANDFLEXIBLE: break;
    case FT_ASCII: break;
    case FT_DATACLOCK: break;
    case FT_FASCN200BIT: break;
    case FT_HIDHONEYWELL: break;
    case FT_GETRONIK40BIT: break;
    case FT_BARIUM_FERRITE_PCSC: break;
    case FT_RAW: break;
    }

    if (cmdstr != "")
    {
        LOG(LogLevel::INFOS) << "Writing format {" << cmdstr << "}";

        std::shared_ptr<GenericTagIdOnDemandCommands> commands =
            std::dynamic_pointer_cast<GenericTagIdOnDemandCommands>(
                getGenericTagChip()->getCommands());
        if (commands)
        {
            std::shared_ptr<IdOnDemandReaderCardAdapter> adapter =
                std::dynamic_pointer_cast<IdOnDemandReaderCardAdapter>(
                    commands->getReaderCardAdapter());
            if (adapter)
            {
                std::shared_ptr<IdOnDemandReaderUnit> idReaderUnit =
                    std::dynamic_pointer_cast<IdOnDemandReaderUnit>(
                        adapter->getDataTransport()->getReaderUnit());

                if (idReaderUnit)
                {
                    adapter->sendCommand(ByteVector(cmdstr.begin(), cmdstr.end()));

                    if (idReaderUnit->write())
                    {
                        if (idReaderUnit->verify())
                        {
                            return true;
                        }
                    }
                }
                else
                {
                    LOG(LogLevel::ERRORS) << "Unable to retrieve the Reader Unit !";
                }
            }
            else
            {
                LOG(LogLevel::ERRORS) << "Unable to retrieve the Card Adapter !";
            }
        }
        else
        {
            LOG(LogLevel::ERRORS) << "Unable to retrieve the Chip !";
        }
    }

    return false;
}
}
