/**
* \file acsacr1222lreaderunit.cpp
* \author Maxime C. <maxime-dev@islog.com>
* \brief ACS ACR 1222L reader unit.
*/

#include <logicalaccess/plugins/readers/iso7816/iso7816resultchecker.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacr1222lreaderunit.hpp>
#include <logicalaccess/plugins/readers/pcsc/readercardadapters/pcscreadercardadapter.hpp>
#include <logicalaccess/bufferhelper.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacr1222lledbuzzerdisplay.hpp>
#include <logicalaccess/plugins/readers/pcsc/readers/acsacr1222llcddisplay.hpp>
#include <logicalaccess/plugins/readers/pcsc/commands/acsacrresultchecker.hpp>
#include <cassert>

namespace logicalaccess
{
ACSACR1222LReaderUnit::ACSACR1222LReaderUnit(const std::string &name)
    : PCSCReaderUnit(name)
{
    d_readerControlReaderCardAdapter.reset(new PCSCReaderCardAdapter());
    d_readerControlReaderCardAdapter->setResultChecker(
        std::make_shared<ISO7816ResultChecker>());

    d_ledBuzzerDisplay.reset(new ACSACR1222LLEDBuzzerDisplay());
    d_lcdDisplay.reset(new ACSACR1222LLCDDisplay());

    d_readerUnitConfig.reset(new ACSACR1222LReaderUnitConfiguration());
}

ACSACR1222LReaderUnit::~ACSACR1222LReaderUnit()
{
}

PCSCReaderUnitType ACSACR1222LReaderUnit::getPCSCType() const
{
    return PCSC_RUT_ACS_ACR_1222L;
}

std::shared_ptr<PCSCReaderCardAdapter>
ACSACR1222LReaderUnit::getReaderControlReaderCardAdapter()
{
    if (d_readerControlReaderCardAdapter)
    {
        if (getDataTransport())
        {
            d_readerControlReaderCardAdapter->setDataTransport(getDataTransport());
        }

        if (d_readerControlReaderCardAdapter->getDataTransport())
        {
            d_readerControlReaderCardAdapter->getDataTransport()->setReaderUnit(
                shared_from_this());
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
        // setDefaultLEDBuzzerBehavior(true, false, false, true);
        d_ledBuzzerDisplay->setGreenLed(true);
        d_ledBuzzerDisplay->setRedLed(false);
    }

    return ret;
}

std::string ACSACR1222LReaderUnit::getReaderSerialNumber()
{
    std::string sn;
    ByteVector res = getReaderControlReaderCardAdapter()->sendAPDUCommand(
        0xE0, 0x00, 0x00, 0x33, 0x00);
    if (res[0] == 0xE1 && res.size() == 21)
    {
        sn = BufferHelper::getHex(ByteVector(res.begin() + 5, res.end()));
    }
    return sn;
}

void ACSACR1222LReaderUnit::setDefaultLEDBuzzerBehavior(bool showPICCPollingStatus,
                                                        bool beepOnCardEvent,
                                                        bool beepOnChipReset,
                                                        bool blinkOnCardOperation)
{
    ByteVector data;
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

void ACSACR1222LReaderUnit::getDefaultLEDBuzzerBehavior(bool &showPICCPollingStatus,
                                                        bool &beepOnCardEvent,
                                                        bool &beepOnChipReset,
                                                        bool &blinkOnCardOperation)
{
    ByteVector res = getReaderControlReaderCardAdapter()->sendAPDUCommand(
        0xE0, 0x00, 0x00, 0x21, 0x00);
    if (res[0] == 0xE1 && res.size() == 6)
    {
        unsigned char value   = res[5];
        showPICCPollingStatus = ((value & 0x02) == 0x02);
        beepOnCardEvent       = ((value & 0x10) == 0x10);
        beepOnChipReset       = ((value & 0x20) == 0x20);
        blinkOnCardOperation  = ((value & 0x80) == 0x80);
    }
}

std::string ACSACR1222LReaderUnit::getFirmwareVersion()
{
    auto ret = getReaderControlReaderCardAdapter()->sendAPDUCommand(0xFF, 0x00, 0x48,
                                                                    0x00, 0x00);
    return std::string(ret.begin(), ret.end());
}

std::shared_ptr<LCDDisplay> ACSACR1222LReaderUnit::getLCDDisplay()
{
    if (sam_used_as_perma_connection_)
        return sam_used_as_perma_connection_->getLCDDisplay();
    return ReaderUnit::getLCDDisplay();
}

std::shared_ptr<LEDBuzzerDisplay> ACSACR1222LReaderUnit::getLEDBuzzerDisplay()
{
    if (sam_used_as_perma_connection_)
        return sam_used_as_perma_connection_->getLEDBuzzerDisplay();
    return ReaderUnit::getLEDBuzzerDisplay();
}

bool ACSACR1222LReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool ret = PCSCReaderUnit::waitRemoval(maxwait);
    if (ret)
    {
        establish_background_connection();
    }
    return ret;
}

bool ACSACR1222LReaderUnit::connectToReader()
{
    bool ret = PCSCReaderUnit::connectToReader();
    if (ret)
    {
        establish_background_connection();
    }
    return ret;
}

void ACSACR1222LReaderUnit::disconnectFromReader()
{
    kill_background_connection();
    PCSCReaderUnit::disconnectFromReader();
}

void ACSACR1222LReaderUnit::establish_background_connection()
{
    kill_background_connection();

    LLA_LOG_CTX("Setting up feedback PCSC connection for ACSACR1222L.");
    auto provider = std::dynamic_pointer_cast<PCSCReaderProvider>(getReaderProvider());
    assert(provider);

    assert(getACSACR1222LConfiguration());
    auto name = getACSACR1222LConfiguration()->getUserFeedbackReader();
    if (name.empty())
    {
        LOG(WARNINGS) << "Cannot fetch name for the reader unit that would be used as "
                      << "a background connection for ACSACR1222L";
        return;
    }
    try
    {
        sam_used_as_perma_connection_ =
            std::dynamic_pointer_cast<PCSCReaderUnit>(provider->createReaderUnit(name));
        assert(sam_used_as_perma_connection_);

        sam_used_as_perma_connection_->setup_pcsc_connection(SC_DIRECT);
    }
    catch (const std::exception &e)
    {
        LOG(ERRORS) << "Failed to establish PCSC connection when establishing "
                    << "background connection for ACSACR1222L: " << e.what();
        sam_used_as_perma_connection_ = nullptr;
        return;
    }

    auto lcd = std::make_shared<ACSACR1222LLCDDisplay>();
    lcd->setReaderCardAdapter(
        sam_used_as_perma_connection_->getDefaultReaderCardAdapter());
    sam_used_as_perma_connection_->setLCDDisplay(lcd);

    auto ledbuzzer = std::make_shared<ACSACR1222LLEDBuzzerDisplay>();
    ledbuzzer->setReaderCardAdapter(
        sam_used_as_perma_connection_->getDefaultReaderCardAdapter());
    sam_used_as_perma_connection_->setLEDBuzzerDisplay(ledbuzzer);
}

void ACSACR1222LReaderUnit::kill_background_connection()
{
    if (sam_used_as_perma_connection_)
    {
        sam_used_as_perma_connection_->disconnect();
        sam_used_as_perma_connection_->disconnectFromReader();
        sam_used_as_perma_connection_->teardown_pcsc_connection();
        sam_used_as_perma_connection_ = nullptr;
    }
}

std::shared_ptr<ACSACR1222LReaderUnitConfiguration>
ACSACR1222LReaderUnit::getACSACR1222LConfiguration()
{
    auto cfg =
        std::dynamic_pointer_cast<ACSACR1222LReaderUnitConfiguration>(getConfiguration());
    if (!cfg)
    {
        LOG(WARNINGS)
            << "Requested ACSACR1222L ReaderUnit Configuration but type mismatch";
    }
    return cfg;
}

std::shared_ptr<ResultChecker> ACSACR1222LReaderUnit::createDefaultResultChecker() const
{
    return std::make_shared<ACSACRResultChecker>();
}
}
