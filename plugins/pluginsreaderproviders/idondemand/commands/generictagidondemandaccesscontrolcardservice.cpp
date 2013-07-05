/**
 * \file generictagidondemandaccesscontrolcardservice.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Generic Tag IdOnDemand Access Control Card service.
 */

#include "generictagidondemandaccesscontrolcardservice.hpp"
#include "generictagidondemandcardprovider.hpp"
#include "generictagcardprovider.hpp"
#include "generictagchip.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"

#include "../readercardadapters/idondemandreadercardadapter.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand26format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand34withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/corporate1000format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37format.hpp"
#include "logicalaccess/services/accesscontrol/formats/wiegand37withfacilityformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/customformat.hpp"
#include "logicalaccess/services/accesscontrol/formats/customformat/numberdatafield.hpp"

#include "logicalaccess/services/accesscontrol/formats/format.hpp"

namespace logicalaccess
{
	GenericTagIdOnDemandAccessControlCardService::GenericTagIdOnDemandAccessControlCardService(boost::shared_ptr<CardProvider> cardProvider)
		: GenericTagAccessControlCardService(cardProvider)
	{
	}

	GenericTagIdOnDemandAccessControlCardService::~GenericTagIdOnDemandAccessControlCardService()
	{
	}

	boost::shared_ptr<Format> GenericTagIdOnDemandAccessControlCardService::readFormat(boost::shared_ptr<Format> /*format*/, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/)
	{
		// Not implemented.
		return boost::shared_ptr<Format>();
	}

	bool GenericTagIdOnDemandAccessControlCardService::writeFormat(boost::shared_ptr<Format> format, boost::shared_ptr<Location> /*location*/, boost::shared_ptr<AccessInfo> /*aiToUse*/, boost::shared_ptr<AccessInfo> /*aiToWrite*/)
	{
		EXCEPTION_ASSERT_WITH_LOG(format, std::invalid_argument, "format to write can't be null.");

		// idOnDemand reader only support specific implemented formats.

		char tmp[64];
		std::string cmdstr = "";

		switch (format->getType())
		{
			case FT_WIEGAND26:
			{
				boost::shared_ptr<Wiegand26Format> wf = boost::dynamic_pointer_cast<Wiegand26Format>(format);
#ifdef __linux__
				sprintf(tmp, "Hid26 %d %llud", wf->getFacilityCode(), wf->getUid());
#else
				sprintf(tmp, "Hid26 %d %d", wf->getFacilityCode(), wf->getUid());
#endif
				cmdstr = std::string(tmp);
				break;
			}
			case FT_WIEGAND34FACILITY:
			{
				boost::shared_ptr<Wiegand34WithFacilityFormat> wf = boost::dynamic_pointer_cast<Wiegand34WithFacilityFormat>(format);
#ifdef __linux__
				sprintf(tmp, "Hid34 %d %llud", wf->getFacilityCode(), wf->getUid());
#else
				sprintf(tmp, "Hid34 %d %d 0", wf->getFacilityCode(), wf->getUid());
#endif
				cmdstr = std::string(tmp);
				break;
			}
			case FT_CORPORATE1000:
			{
				boost::shared_ptr<Corporate1000Format> wf = boost::dynamic_pointer_cast<Corporate1000Format>(format);
#ifdef __linux__
				sprintf(tmp, "Hid35 %d %llud 3", wf->getCompanyCode(), wf->getUid());
#else
				sprintf(tmp, "Hid35 %d %d 3", wf->getCompanyCode(), wf->getUid());
#endif
				cmdstr = std::string(tmp);
				break;
			}
			case FT_WIEGAND37FACILITY:
			{
				boost::shared_ptr<Wiegand37WithFacilityFormat> wf = boost::dynamic_pointer_cast<Wiegand37WithFacilityFormat>(format);
#ifdef __linux__
				sprintf(tmp, "Hid37 %d %lld 2", wf->getFacilityCode(), wf->getUid());
#else
				sprintf(tmp, "Hid37 %d %d 2", wf->getFacilityCode(), wf->getUid());
#endif
				cmdstr = std::string(tmp);
				break;
			}
			case FT_WIEGAND37:
			{
				boost::shared_ptr<Wiegand37Format> wf = boost::dynamic_pointer_cast<Wiegand37Format>(format);
#ifdef __linux__
				sprintf(tmp, "Hid37 0 %llud 1", wf->getUid());
#else
				sprintf(tmp, "Hid37 0 %d 1", wf->getUid());
#endif
				cmdstr = std::string(tmp);
				break;
			}
			case FT_CUSTOM:
			{
				boost::shared_ptr<CustomFormat> wf = boost::dynamic_pointer_cast<CustomFormat>(format);
				boost::shared_ptr<NumberDataField> fieldUid = boost::dynamic_pointer_cast<NumberDataField>(wf->getFieldFromName("Uid"));
				boost::shared_ptr<NumberDataField> fieldFacility = boost::dynamic_pointer_cast<NumberDataField>(wf->getFieldFromName("Facility Code"));

				if (format->getName() == "Wiegand 32")
				{
#ifdef __linux__
					sprintf(tmp, "Hid32 %llud %llud", fieldFacility->getValue(), fieldUid->getValue());
#else
					sprintf(tmp, "Hid32 %d %d", fieldFacility->getValue(), fieldUid->getValue());
#endif
					cmdstr = std::string(tmp);
				}
				else if (format->getName() == "Wiegand 37 With 13-bit Facility")
				{
#ifdef __linux__
					sprintf(tmp, "Hid37 %llud %llud 3", fieldFacility->getValue(), fieldUid->getValue());
#else
					sprintf(tmp, "Hid37 %d %d 3", fieldFacility->getValue(), fieldUid->getValue());
#endif
					cmdstr = std::string(tmp);
				}
				else if (format->getName() == "Wiegand 40")
				{
#ifdef __linux__
					sprintf(tmp, "Hid40 %llud %llud", fieldFacility->getValue(), fieldUid->getValue());
#else
					sprintf(tmp, "Hid40 %d %d", fieldFacility->getValue(), fieldUid->getValue());
#endif
					cmdstr = std::string(tmp);
				}
				else if (format->getName() == "Wiegand 42 (HID10313C compatible)")
				{
#ifdef __linux__
					sprintf(tmp, "Hid42 %llud %llud", fieldFacility->getValue(), fieldUid->getValue());
#else
					sprintf(tmp, "Hid42 %d %d", fieldFacility->getValue(), fieldUid->getValue());
#endif
					cmdstr = std::string(tmp);
				}
				else if (format->getName() == "Honeywell")
				{
#ifdef __linux__
					sprintf(tmp, "Honeywell %llud", fieldUid->getValue());
#else
					sprintf(tmp, "Honeywell %d", fieldUid->getValue());
#endif
					cmdstr = std::string(tmp);
				}

				break;
			}
		case FT_UNKNOWN:
		  break;
		case FT_WIEGAND34:
		  break;
		case FT_WIEGANDFLEXIBLE:
		  break;
		case FT_ASCII:
		  break;
		case FT_DATACLOCK:
		  break;
		case FT_FASCN200BIT:
		  break;
		case FT_HIDHONEYWELL:
		  break;
		case FT_GETRONIK40BIT:
		  break;
		case FT_BARIUM_FERRITE_PCSC:
		  break;
		case FT_RAW:
		  break;
		}

		if (cmdstr != "")
		{
			INFO_("Writing format {%s}", cmdstr.c_str());

			boost::shared_ptr<GenericTagIdOnDemandCardProvider> idCardProvider = boost::dynamic_pointer_cast<GenericTagIdOnDemandCardProvider>(getCardProvider());
			if (idCardProvider)
			{
				boost::shared_ptr<IdOnDemandReaderCardAdapter> adapter = boost::dynamic_pointer_cast<IdOnDemandReaderCardAdapter>(idCardProvider->getReaderCardAdapter());
				if (adapter)
				{
					boost::shared_ptr<IdOnDemandReaderUnit> idReaderUnit = boost::dynamic_pointer_cast<IdOnDemandReaderUnit>(adapter->getDataTransport()->getReaderUnit());

					if (idReaderUnit)
					{
						adapter->sendCommand(std::vector<unsigned char>(cmdstr.begin(), cmdstr.end()));

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
						ERROR_SIMPLE_("Unable to retrieve the Reader Unit !");
					}
				}
				else
				{
					ERROR_SIMPLE_("Unable to retrieve the Card Adapter !");
				}
			}
			else
			{
				ERROR_SIMPLE_("Unable to retrieve the Card Provider !");
			}
		}

		return false;
	}
}

