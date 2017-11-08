/**
 * \file rfideasreaderunit.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief RFIDeas reader unit.
 */

#include <logicalaccess/plugins/readers/rfideas/rfideasreaderunit.hpp>
#include <logicalaccess/cards/readercardadapter.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

#include <logicalaccess/plugins/readers/rfideas/rfideasreaderprovider.hpp>
#include <logicalaccess/services/accesscontrol/cardsformatcomposite.hpp>
#include <logicalaccess/cards/chip.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <logicalaccess/dynlibrary/librarymanager.hpp>
#include <logicalaccess/dynlibrary/idynlibrary.hpp>
#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>

namespace logicalaccess
{
RFIDeasReaderUnit::RFIDeasReaderUnit()
    : ReaderUnit(READER_RFIDEAS)
{
    d_lastTagIdBitsLength = 0x00;
    d_deviceId            = 0x00;
    d_readerUnitConfig.reset(new RFIDeasReaderUnitConfiguration());
    d_card_type = CHIP_GENERICTAG;

    try
    {
        boost::property_tree::ptree pt;
        read_xml(
            (boost::filesystem::current_path().string() + "/RFIDeasReaderUnit.config"),
            pt);
        d_card_type = pt.get("config.cardType", CHIP_GENERICTAG);
    }
    catch (...)
    {
    }

    isCOMConnection = false;

    initExternFct();
}

RFIDeasReaderUnit::~RFIDeasReaderUnit()
{
    uninitExternFct();
    uninitReaderCnx();
}

void RFIDeasReaderUnit::initExternFct()
{
#ifdef _WINDOWS
    hWejAPIDLL = (HMODULE)LoadLibrary("pcProxAPI.DLL");
    EXCEPTION_ASSERT_WITH_LOG(hWejAPIDLL != NULL, LibLogicalAccessException,
                              "Can't load the RFIDeas library");

    fnGetLibVersion   = (GetLibVersion)GetProcAddress(hWejAPIDLL, "GetLibVersion");
    fnCOMConnect      = (ComConnect)GetProcAddress(hWejAPIDLL, "ComConnect");
    fnCOMDisconnect   = (ComDisconnect)GetProcAddress(hWejAPIDLL, "ComDisconnect");
    fnSetComSrchRange = (SetComSrchRange)GetProcAddress(hWejAPIDLL, "SetComSrchRange");
    fnUSBConnect      = (USBConnect)GetProcAddress(hWejAPIDLL, "USBConnect");
    fnUSBDisconnect   = (USBDisconnect)GetProcAddress(hWejAPIDLL, "USBDisconnect");
    fnGetLastLibErr   = (GetLastLibErr)GetProcAddress(hWejAPIDLL, "GetLastLibErr");
    fnReadCfg         = (ReadCfg)GetProcAddress(hWejAPIDLL, "ReadCfg");
    fnWriteCfg        = (WriteCfg)GetProcAddress(hWejAPIDLL, "WriteCfg");
    fnGetFlags        = (GetFlags)GetProcAddress(hWejAPIDLL, "GetFlags");
    fnSetFlags        = (SetFlags)GetProcAddress(hWejAPIDLL, "SetFlags");
    fnGetActiveID     = (GetActiveID)GetProcAddress(hWejAPIDLL, "GetActiveID");
    fnGetActiveID32   = (GetActiveID32)GetProcAddress(hWejAPIDLL, "GetActiveID32");
    fnGetDevCnt       = (GetDevCnt)GetProcAddress(hWejAPIDLL, "GetDevCnt");
    fnSetActDev       = (SetActDev)GetProcAddress(hWejAPIDLL, "SetActDev");
    fnSetLUID         = (SetLUID)GetProcAddress(hWejAPIDLL, "SetLUID");
    fnGetQueuedID     = (GetQueuedID)GetProcAddress(hWejAPIDLL, "GetQueuedID");
    fnGetQueuedID_index =
        (GetQueuedID_index)GetProcAddress(hWejAPIDLL, "GetQueuedID_index");
    fnSetConnectProduct =
        (SetConnectProduct)GetProcAddress(hWejAPIDLL, "SetConnectProduct");
    fnSetDevTypeSrch = (SetDevTypeSrch)GetProcAddress(hWejAPIDLL, "SetDevTypeSrch");

    if ((fnGetLibVersion == nullptr) || (fnSetComSrchRange == nullptr) ||
        (fnCOMConnect == nullptr) || (fnCOMDisconnect == nullptr) ||
        (fnUSBConnect == nullptr) || (fnUSBDisconnect == nullptr) ||
        (fnGetLastLibErr == nullptr) || (fnReadCfg == nullptr) ||
        (fnWriteCfg == nullptr) || (fnGetFlags == nullptr) || (fnSetFlags == nullptr) ||
        (fnGetActiveID == nullptr) || (fnGetActiveID32 == nullptr) ||
        (fnGetDevCnt == nullptr) || (fnSetActDev == nullptr) || (fnSetLUID == nullptr) ||
        (fnGetQueuedID == nullptr) || (fnGetQueuedID_index == nullptr) ||
        (fnSetConnectProduct == nullptr) || (fnSetDevTypeSrch == nullptr))
    {
        uninitExternFct();
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Can't initialize RFIDeas library functions pointer");
    }
#endif
}

void RFIDeasReaderUnit::uninitExternFct()
{
#ifdef _WINDOWS
    FreeLibrary(hWejAPIDLL);
    hWejAPIDLL = nullptr;
#endif
}

void RFIDeasReaderUnit::initReaderCnx()
{
#ifdef _WINDOWS
    LOG(LogLevel::INFOS) << "SetConnectProduct returned " << std::hex
                         << fnSetConnectProduct(PRODUCT_PCPROX) << std::dec;
    LOG(LogLevel::INFOS) << "SetDevTypeSrch returned " << std::hex
                         << fnSetDevTypeSrch(PRXDEVTYP_USB) << std::dec;

    BSHRT hr = fnUSBConnect(&d_deviceId);
    LOG(LogLevel::INFOS) << "USBConnect returned " << std::hex << hr << std::dec;
    if (hr == 0)
    {
        LOG(LogLevel::INFOS) << "SetComSrchRange returned " << std::hex
                             << fnSetComSrchRange(1, 15) << std::dec;
        hr = fnCOMConnect(&d_deviceId);
        LOG(LogLevel::INFOS) << "COMConnect returned " << std::hex << hr << std::dec;
        EXCEPTION_ASSERT_WITH_LOG(
            hr != 0, LibLogicalAccessException,
            "Can't connect to the RFIDeas device. Please be sure a reader is plugged");
        isCOMConnection = true;
    }
    else
    {
        isCOMConnection = false;
    }
#endif
}

void RFIDeasReaderUnit::uninitReaderCnx()
{
#ifdef _WINDOWS
    if (d_deviceId != 0)
    {
        if (isCOMConnection)
        {
            BSHRT hr = fnCOMDisconnect();
            LOG(LogLevel::INFOS) << "COMDisconnect returned " << std::hex << hr
                                 << std::dec;
        }
        else
        {
            BSHRT hr = fnUSBDisconnect();
            LOG(LogLevel::INFOS) << "USBDisconnect returned " << std::hex << hr
                                 << std::dec;
        }
    }
#endif
    d_deviceId = 0;
}

std::string RFIDeasReaderUnit::getName() const
{
    return std::string();
}

std::string RFIDeasReaderUnit::getConnectedName()
{
    char conv[64];
    sprintf(conv, "RFIDeas Device ID: %ld", d_deviceId);

    return std::string(conv);
}

void RFIDeasReaderUnit::setCardType(std::string cardType)
{
    d_card_type = cardType;
}

bool RFIDeasReaderUnit::waitInsertion(unsigned int maxwait)
{
    bool inserted            = false;
    unsigned int currentWait = 0;

    do
    {
        ByteVector tagid = getTagId();
        if (tagid.size() > 0)
        {
            d_insertedChip = ReaderUnit::createChip(
                (d_card_type == CHIP_UNKNOWN) ? CHIP_GENERICTAG : d_card_type, tagid);
            inserted = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        currentWait += 250;
    } while (!inserted && (maxwait == 0 || currentWait < maxwait));

    return inserted;
}

bool RFIDeasReaderUnit::waitRemoval(unsigned int maxwait)
{
    bool removed = false;

    if (d_insertedChip)
    {
        unsigned int currentWait = 0;
        do
        {
            ByteVector tagid = getTagId();
            if (tagid.size() > 0)
            {
                if (tagid != d_insertedChip->getChipIdentifier())
                {
                    d_insertedChip.reset();
                    removed = true;
                }
            }
            else
            {
                d_insertedChip.reset();
                removed = true;
            }

            if (!removed)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                currentWait += 250;
            }
        } while (!removed && (maxwait == 0 || currentWait < maxwait));
    }

    return removed;
}

bool RFIDeasReaderUnit::connect()
{
    return true;
}

void RFIDeasReaderUnit::disconnect()
{
}

bool RFIDeasReaderUnit::connectToReader()
{
    initReaderCnx();
    return true;
}

void RFIDeasReaderUnit::disconnectFromReader()
{
    uninitReaderCnx();
}

ByteVector RFIDeasReaderUnit::getTagId()
{
    ByteVector tagid;

#ifdef _WINDOWS
    unsigned char idbuf[32];
    memset(idbuf, 0x00, sizeof(idbuf));
    d_lastTagIdBitsLength = fnGetActiveID(idbuf, sizeof(idbuf));
    if (d_lastTagIdBitsLength > 16)
    {
        int bytes = (d_lastTagIdBitsLength + 7) / 8;

        tagid.insert(tagid.end(), idbuf, idbuf + bytes);
        // Swap LSByte => MSByte
        reverse(tagid.begin(), tagid.end());
    }
    else
    {
        long err = fnGetLastLibErr();
        if (err > 0)
        {
            LOG(LogLevel::WARNINGS) << "An error occured when getting the active ID ("
                                    << err << ")";
            LOG(LogLevel::INFOS) << "Disconnecting from the reader...";
            disconnectFromReader();
            LOG(LogLevel::INFOS) << "Reconnecting to the reader...";
            connectToReader();
        }
    }
#endif

    return tagid;
}

std::shared_ptr<Chip> RFIDeasReaderUnit::createChip(std::string type)
{
    std::shared_ptr<Chip> chip = ReaderUnit::createChip(type);
    setTagIdBitsLengthFct setagfct;

    if (chip)
    {
        std::shared_ptr<ReaderCardAdapter> rca;

        if (type == CHIP_GENERICTAG)
        {
            rca = getDefaultReaderCardAdapter();

            *(void **)(&setagfct) = LibraryManager::getInstance()->getFctFromName(
                "setTagIdBitsLengthOfGenericTagChip", LibraryManager::CARDS_TYPE);
            setagfct(&chip, d_lastTagIdBitsLength);
        }
        else
            return chip;

        if (rca)
        {
            rca->setDataTransport(getDataTransport());
        }
    }
    return chip;
}

std::shared_ptr<Chip> RFIDeasReaderUnit::getSingleChip()
{
    std::shared_ptr<Chip> chip = d_insertedChip;
    return chip;
}

std::vector<std::shared_ptr<Chip>> RFIDeasReaderUnit::getChipList()
{
    std::vector<std::shared_ptr<Chip>> chipList;
    std::shared_ptr<Chip> singleChip = getSingleChip();
    if (singleChip)
    {
        chipList.push_back(singleChip);
    }
    return chipList;
}

std::string RFIDeasReaderUnit::getReaderSerialNumber()
{
    return "";
}

bool RFIDeasReaderUnit::isConnected()
{
    return (bool)d_insertedChip;
}

void RFIDeasReaderUnit::serialize(boost::property_tree::ptree &parentNode)
{
    boost::property_tree::ptree node;

    node.put("<xmlattr>.type", getReaderProvider()->getRPType());
    d_readerUnitConfig->serialize(node);

    parentNode.add_child(getDefaultXmlNodeName(), node);
}

void RFIDeasReaderUnit::unSerialize(boost::property_tree::ptree &node)
{
    d_readerUnitConfig->unSerialize(
        node.get_child(d_readerUnitConfig->getDefaultXmlNodeName()));
}

std::shared_ptr<RFIDeasReaderProvider> RFIDeasReaderUnit::getRFIDeasReaderProvider() const
{
    return std::dynamic_pointer_cast<RFIDeasReaderProvider>(getReaderProvider());
}

std::shared_ptr<RFIDeasReaderUnit> RFIDeasReaderUnit::getSingletonInstance()
{
    static std::shared_ptr<RFIDeasReaderUnit> instance;
    if (!instance)
    {
        instance.reset(new RFIDeasReaderUnit());
    }
    return instance;
}
}