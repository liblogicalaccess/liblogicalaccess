/**
 * \file iso14443readercommunication.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief ISO14443-3 Reader communication.
 */

#include "logicalaccess/readerproviders/iso14443readercommunication.hpp"


namespace logicalaccess
{	
	ISO14443ReaderCommunication::ISO14443ReaderCommunication() :
		ISO14443AReaderCommunication(), ISO14443BReaderCommunication(), d_rcMode(IM_UNKNOWN)
	{

	}

	std::vector<unsigned char> ISO14443ReaderCommunication::request()
	{
		std::vector<unsigned char> ret;

		switch (d_rcMode)
		{
		case IM_ISO14443A:
			ret = requestA();
			break;

		case IM_ISO14443B:
			ret = requestB();
			break;

		case IM_UNKNOWN:
			try
			{
				ret = requestA();
				d_rcMode = IM_ISO14443A;
			}
			catch(std::exception&)
			{
				ret = requestB();
				d_rcMode = IM_ISO14443B;
			}

			break;

		default:
			break;
		}

		return ret;
	}

	std::vector<unsigned char> ISO14443ReaderCommunication::anticollision()
	{
		std::vector<unsigned char> ret;

		switch (d_rcMode)
		{
		case IM_ISO14443A:
			ret = anticollisionA();
			break;

		case IM_ISO14443B:
			ret = anticollisionB();
			break;

		default:
			break;
		}

		return ret;
	}

	void ISO14443ReaderCommunication::selectIso(const std::vector<unsigned char>& /*uid*/)
	{
		switch (d_rcMode)
		{
			case IM_ISO14443A:
				requestATS();
				break;

			case IM_ISO14443B:
				attrib();
				break;

			default:
				break;
		}
	}

	void ISO14443ReaderCommunication::halt()
	{
		switch (d_rcMode)
		{
		case IM_ISO14443A:
			haltA();
			break;

		case IM_ISO14443B:
			haltB();
			break;

		default:
			break;
		}
	}
}
