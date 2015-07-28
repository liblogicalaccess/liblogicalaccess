/**
* \file acsacr1222lreaderunit.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L reader unit.
*/

#include "iso7816resultchecker.hpp"
#include "../readers/acsacr1222lreaderunit.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"
#include "acsacr1222lledbuzzerdisplay.hpp"
#include "acsacr1222llcddisplay.hpp"

namespace logicalaccess
{
    ACSACR1222LReaderUnit::ACSACR1222LReaderUnit(const std::string& name)
        : PCSCReaderUnit(name)
    {
        d_readerControlReaderCardAdapter.reset(new PCSCReaderCardAdapter());
        d_readerControlReaderCardAdapter->setResultChecker(std::make_shared<ISO7816ResultChecker>());

        d_ledBuzzerDisplay.reset(new ACSACR1222LLEDBuzzerDisplay());
        d_lcdDisplay.reset(new ACSACR1222LLCDDisplay());
    }

    ACSACR1222LReaderUnit::~ACSACR1222LReaderUnit()
    {
    }

    PCSCReaderUnitType ACSACR1222LReaderUnit::getPCSCType() const
    {
        return PCSC_RUT_ACS_ACR_1222L;
    }

    std::shared_ptr<PCSCReaderCardAdapter> ACSACR1222LReaderUnit::getReaderControlReaderCardAdapter()
    {
        if (d_readerControlReaderCardAdapter)
        {
            if (getDataTransport())
            {
                d_readerControlReaderCardAdapter->setDataTransport(getDataTransport());
            }

            if (d_readerControlReaderCardAdapter->getDataTransport())
            {
                d_readerControlReaderCardAdapter->getDataTransport()->setReaderUnit(shared_from_this());
            }
        }
        return d_readerControlReaderCardAdapter;
    }

    bool ACSACR1222LReaderUnit::connect()
    {
        bool ret = PCSCReaderUnit::connect();

        // Finish setting up the ledbuzzer object.
        // We cannot do that in constructor because passing the reader unit to the
        // datatransport requires calling shared_from_this(), which cannot be done
        // from constructor.
        d_ledBuzzerDisplay->setReaderCardAdapter(getReaderControlReaderCardAdapter());

        if (ret)
        {
            // The firmware doesn't seem to implement this, despite what the doc
            // is saying.
            //setDefaultLEDBuzzerBehavior(true, false, false, true);
            d_ledBuzzerDisplay->setGreenLed(true);
            d_ledBuzzerDisplay->setRedLed(false);
        }

        return ret;
    }

    std::string ACSACR1222LReaderUnit::getReaderSerialNumber()
    {
        std::string sn;
        std::vector<unsigned char> res = getReaderControlReaderCardAdapter()->sendAPDUCommand(0xE0, 0x00, 0x00, 0x33, 0x00);
        if (res[0] == 0xE1 && res.size() == 21)
        {
            sn = BufferHelper::getHex(std::vector<unsigned char>(res.begin() + 5, res.end()));
        }
        return sn;
    }

    void ACSACR1222LReaderUnit::setDefaultLEDBuzzerBehavior(bool showPICCPollingStatus, bool beepOnCardEvent, bool beepOnChipReset, bool blinkOnCardOperation)
    {
        std::vector<unsigned char> data;
        unsigned char value = 0x00;
        if (showPICCPollingStatus)
            value |= 0x02;
        if (beepOnCardEvent)
            value |= 0x10;
        if (beepOnChipReset)
            value |= 0x20;
        if (blinkOnCardOperation)
            value |= 0x80;
        data.push_back(value);

        getReaderControlReaderCardAdapter()->sendAPDUCommand(0xE0, 0x00, 0x00, 0x21, data);
    }

    void ACSACR1222LReaderUnit::getDefaultLEDBuzzerBehavior(bool& showPICCPollingStatus, bool& beepOnCardEvent, bool& beepOnChipReset, bool& blinkOnCardOperation)
    {
        std::vector<unsigned char> res = getReaderControlReaderCardAdapter()->sendAPDUCommand(0xE0, 0x00, 0x00, 0x21, 0x00);
        if (res[0] == 0xE1 && res.size() == 6)
        {
            unsigned char value = res[5];
            showPICCPollingStatus = ((value & 0x02) == 0x02);
            beepOnCardEvent = ((value & 0x10) == 0x10);
            beepOnChipReset = ((value & 0x20) == 0x20);
            blinkOnCardOperation = ((value & 0x80) == 0x80);
        }
    }

    std::string ACSACR1222LReaderUnit::getFirmwareVersion()
    {
        auto ret = getReaderControlReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x48, 0x00, 0x00);
        return std::string(ret.begin(), ret.end());
    }
}
