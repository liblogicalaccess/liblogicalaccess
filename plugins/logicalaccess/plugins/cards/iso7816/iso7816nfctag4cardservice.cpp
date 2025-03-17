/**
 * \file iso7816nfctag4cardservice.cpp
 * \author Adrien J. <adrien-dev@islog.com>
 * \brief ISO7816 NFC Tag Type 4 card service.
 */

#include <logicalaccess/plugins/llacommon/logs.hpp>
#include <logicalaccess/plugins/cards/iso7816/iso7816nfctag4cardservice.hpp>
#include <logicalaccess/services/nfctag/ndefmessage.hpp>
#include <logicalaccess/myexception.hpp>

#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>

namespace logicalaccess
{
void ISO7816NFCTag4CardService::writeCapabilityContainer(unsigned short isoFID,
                                                         unsigned short isoFIDNDEFFile,
                                                         unsigned short NDEFFileSize)
{
    auto iso7816command = getISO7816Commands();

    ByteVector CC;
    CC.push_back(0x00); // CCLEN
    CC.push_back(0x0f);

    CC.push_back(0x20); // Mapping version

    CC.push_back(0x00); // maximum data size can be read with READ BINARY (Default Value
                        // for DESFire)
    CC.push_back(0x3A);

    CC.push_back(0x00); // maximum data size can be write with UPDATE BINARY (Default
                        // Value for DESFire)
    CC.push_back(0x34);

    CC.push_back(0x04); // Tag , FileControl TLV (4 = NDEFfile)
    CC.push_back(0x06); // Length, FileControl TLV (6 = 6 bytes of data for this tag)
    CC.push_back((isoFIDNDEFFile & 0xff00) >>
                 8);                     // file identifier 0xe104-0xe10f or 0xe100/0xe101
    CC.push_back(isoFIDNDEFFile & 0xff); // file identifier
    CC.push_back((NDEFFileSize & 0xff00) >> 8); // MemorySize
    CC.push_back(NDEFFileSize & 0xff);          // MemorySize
    CC.push_back(0x00);                         // read access
    CC.push_back(0x00);                         // write access

    iso7816command->selectFile(P1_SELECT_MF_DF_EF, P2_FIRST_RECORD, isoFID);
    iso7816command->updateBinary(CC, 0, isoFID);
}

void ISO7816NFCTag4CardService::writeNDEFFile(ByteVector recordsData,
                                              unsigned short isoFIDNDEFFile)
{
    auto iso7816cmd = getISO7816Commands();
    ByteVector data;

    data.push_back(static_cast<unsigned char>((recordsData.size() >> 8) & 0xff)); // NLEN
    data.push_back(static_cast<unsigned char>(recordsData.size() & 0xff)); // NDEF Length
    data.insert(data.end(), recordsData.begin(), recordsData.end());

    iso7816cmd->selectFile(P1_SELECT_MF_DF_EF, P2_FIRST_RECORD, isoFIDNDEFFile);
    iso7816cmd->updateBinary(data, 0, isoFIDNDEFFile);
}

void ISO7816NFCTag4CardService::writeNDEFFile(std::shared_ptr<NdefMessage> records,
                                              unsigned short isoFIDNDEFFile)
{
    writeNDEFFile(records->encode(), isoFIDNDEFFile);
}

std::shared_ptr<NdefMessage>
ISO7816NFCTag4CardService::readNDEFFile(const ByteVector &appDFName,
                                        unsigned short isoFIDNDEFFile)
{
    std::shared_ptr<ISO7816Commands> iso7816command = getISO7816Commands();

    size_t length = 0x02;

	iso7816command->selectFile(P1_SELECT_BY_DFNAME, P2_FIRST_RECORD, appDFName);
	iso7816command->selectFile(P1_SELECT_MF_DF_EF, P2_FIRST_RECORD, isoFIDNDEFFile);
    ByteVector data = iso7816command->readBinary(length, 0, isoFIDNDEFFile);

    if (length != 0x02)
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Cannot read NDEF Length");
    length = (data[0] << 8) | data[1];
    data   = iso7816command->readBinary(length, 2, isoFIDNDEFFile);
    return std::make_shared<NdefMessage>(data);
}

std::shared_ptr<NdefMessage> ISO7816NFCTag4CardService::readNDEF()
{
    return readNDEFFile();
}

std::shared_ptr<ISO7816Commands> ISO7816NFCTag4CardService::getISO7816Commands() const
{
    auto chip = std::dynamic_pointer_cast<ISO7816Chip>(getChip());
    if (chip == nullptr)
    {
        return nullptr;
    }
    return std::dynamic_pointer_cast<ISO7816Commands>(chip->getCommands());
}
}
