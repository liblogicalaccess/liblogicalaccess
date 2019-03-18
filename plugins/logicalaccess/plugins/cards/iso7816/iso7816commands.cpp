/**
 * \file iso7816commands.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO7816 commands.
 */

#include <logicalaccess/plugins/cards/iso7816/iso7816commands.hpp>

namespace logicalaccess
{
ISO7816Commands::~ISO7816Commands() {}

void ISO7816Commands::selectFile(unsigned short efid)
{
    selectFile(P1_SELECT_MF_DF_EF, P2_RETURN_NO_FCI, efid);
}

void ISO7816Commands::selectFile(ISOSelectFileP1 p1, ISOSelectFileP2 p2,
                                        unsigned short efid)
{
    unsigned char data[2];
    data[0] = 0xff & (efid >> 8);
    data[1] = 0xff & efid;

    selectFile(p1, p2, ByteVector(data, data + sizeof(data)));
}

void ISO7816Commands::selectFile(const ByteVector &dfname)
{
    selectFile(P1_SELECT_BY_DFNAME, P2_RETURN_NO_FCI, dfname);
}

ByteVector ISO7816Commands::getDataList(int64_t dataObject,
                                        unsigned short efid, unsigned char listtag,
                                        size_t length)
{
    auto tlv = std::make_shared<TLV>(listtag);
    ByteVector v;
    if (dataObject > 0xffff)
	{
		v.push_back(static_cast<unsigned char>(0xFF & (dataObject >> 16)));
	}
    v.push_back(static_cast<unsigned char>(0xFF & (dataObject >> 8)));
    v.push_back(static_cast<unsigned char>(0xFF & dataObject));
    tlv->value(v);

    return getDataList(tlv, length, efid);
}

ByteVector ISO7816Commands::getDataList(TLVPtr tlv, size_t length, unsigned short efid)
{
    return getDataList(tlv->value(), length, efid);
}

void ISO7816Commands::removeApplication(TLVPtr tlv, ISORemoveApplicationP1 p1)
{
    removeApplication(tlv->value(), p1);
}

ByteVector ISO7816Commands::applicationManagementRequest(
    TLVPtr tlv, ISOApplicationManagementRequestP1 p1, unsigned char p2)
{
    return applicationManagementRequest(tlv->value(), p1, p2);
}
}