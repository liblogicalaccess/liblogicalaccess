/**
 * \file pcscdatatransport.cpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief PC/SC data transport.
 */

#include "pcscdatatransport.hpp"
#include "logicalaccess/cards/readercardadapter.hpp"
#include "logicalaccess/bufferhelper.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace logicalaccess
{
	PCSCDataTransport::PCSCDataTransport()
	{
		d_isConnected = false;
	}

	PCSCDataTransport::~PCSCDataTransport()
	{

	}

	bool PCSCDataTransport::connect()
	{
		d_isConnected = true;
		return true;
	}

	void PCSCDataTransport::disconnect()
	{
		d_isConnected = false;
	}

	bool PCSCDataTransport::isConnected()
	{
		return d_isConnected;
	}

	void PCSCDataTransport::send(const std::vector<unsigned char>& data)
	{
		d_response.clear();

		if (data.size() > 0)
		{
			unsigned char returnedData[255];
			ULONG ulNoOfDataReceived = sizeof(returnedData);
			LPCSCARD_IO_REQUEST ior = NULL;
			switch (getPCSCReaderUnit()->getActiveProtocol())
			{
			case SCARD_PROTOCOL_T0:
				ior = SCARD_PCI_T0;
				break;

			case SCARD_PROTOCOL_T1:
				ior = SCARD_PCI_T1;
				break;

			case SCARD_PROTOCOL_RAW:
				ior = SCARD_PCI_RAW;
				break;
			}

			COM_("APDU command: %s", BufferHelper::getHex(data).c_str());

			unsigned int errorFlag = SCardTransmit(getPCSCReaderUnit()->getHandle(), ior, &data[0], static_cast<DWORD>(data.size()), NULL, returnedData, &ulNoOfDataReceived);

			CheckCardError(errorFlag);
			d_response = std::vector<unsigned char>(returnedData, returnedData + ulNoOfDataReceived);
		}
	}

	std::vector<unsigned char> PCSCDataTransport::receive(long int /*timeout*/)
	{
		std::vector<unsigned char> r = d_response;
		COM_("APDU response: %s", BufferHelper::getHex(r).c_str());
		CheckResult(&r[0], r.size());

		d_response.clear();
		return r;
	}

	std::string PCSCDataTransport::getName() const
	{
		return getPCSCReaderUnit()->getName();
	}

	void PCSCDataTransport::serialize(boost::property_tree::ptree& parentNode)
	{
		boost::property_tree::ptree node;

		node.put("<xmlattr>.type", getTransportType());
		parentNode.add_child(getDefaultXmlNodeName(), node);
	}

	void PCSCDataTransport::unSerialize(boost::property_tree::ptree& /*node*/)
	{
		
	}

	std::string PCSCDataTransport::getDefaultXmlNodeName() const
	{
		return "PCSCDataTransport";
	}

	void PCSCDataTransport::CheckCardError(unsigned int errorFlag) const
	{
		if (errorFlag != SCARD_S_SUCCESS)
		{
			char conv[64];
			std::string msg = std::string("Card transmit error : ");
			sprintf(conv, "%x", errorFlag);
			msg += std::string(conv);
			msg += std::string(". ");
			switch ((errorFlag | 0x80100000L))
			{
			case SCARD_F_INTERNAL_ERROR:
				msg += std::string("An internal consistency check failed.");
				break;

			case SCARD_E_CANCELLED:
				msg += std::string("The action was cancelled by a Cancel request.");
				break;

			case SCARD_E_INVALID_HANDLE:
				msg += std::string("The supplied handle was invalid.");
				break;

				case SCARD_E_INVALID_PARAMETER:
				msg += std::string("One or more of the supplied parameters could not be properly interpreted.");
				break;

			case SCARD_E_INVALID_TARGET:
				msg += std::string("Registry startup information is missing or invalid.");
				break;

			case SCARD_E_NO_MEMORY:
				msg += std::string("Not enough memory available to complete this command.");
				break;

			case SCARD_F_WAITED_TOO_LONG:
				msg += std::string("An internal consistency timer has expired.");
				break;

			case SCARD_E_INSUFFICIENT_BUFFER:
				msg += std::string("The data buffer to receive returned data is too small for the returned data.");
				break;

			case SCARD_E_UNKNOWN_READER:
				msg += std::string("The specified reader name is not recognized.");
				break;

			case SCARD_E_TIMEOUT:
				msg += std::string("The user-specified timeout value has expired.");
				break;

			case SCARD_E_SHARING_VIOLATION:
				msg += std::string("The smart card cannot be accessed because of other connections outstanding.");
				break;

			case SCARD_E_NO_SMARTCARD:
				msg += std::string("The operation requires a Smart Card, but no Smart Card is currently in the device.");
				break;

			case SCARD_E_UNKNOWN_CARD:
				msg += std::string("The specified smart card name is not recognized.");
				break;

			case SCARD_E_CANT_DISPOSE:
				msg += std::string("The system could not dispose of the media in the requested manner.");
				break;

			case SCARD_E_PROTO_MISMATCH:
				msg += std::string("The requested protocols are incompatible with the protocol currently in use with the smart card.");
				break;

			case SCARD_E_INVALID_VALUE:
				msg += std::string("One or more of the supplied parameters values could not be properly interpreted.");
				break;

			case SCARD_E_SYSTEM_CANCELLED:
				msg += std::string("The action was cancelled by the system, presumably to log off or shut down.");
				break;

			case SCARD_F_COMM_ERROR:
				msg += std::string("An internal communications error has been detected.");
				break;

			case SCARD_F_UNKNOWN_ERROR:
				msg += std::string("An internal error has been detected, but the source is unknown.");
				break;

			case SCARD_E_INVALID_ATR:
				msg += std::string("An ATR obtained from the registry is not a valid ATR string.");
				break;

			case SCARD_E_NOT_TRANSACTED:
				msg += std::string("An attempt was made to end a non-existent transaction.");
				break;

			case SCARD_E_READER_UNAVAILABLE:
				msg += std::string("The specified reader is not currently available for use.");
				break;

			case SCARD_E_PCI_TOO_SMALL:
				msg += std::string("The PCI Receive buffer was too small.");
				break;

			case SCARD_E_READER_UNSUPPORTED:
				msg += std::string("The reader driver does not meet minimal requirements for support.");
				break;

			case SCARD_E_DUPLICATE_READER:
				msg += std::string("The reader driver did not produce a unique reader name.");
				break;

			case SCARD_E_CARD_UNSUPPORTED:
				msg += std::string("The smart card does not meet minimal requirements for support.");
				break;

			case SCARD_E_NO_SERVICE:
				msg += std::string("The Smart card resource manager is not running.");
				break;

			case SCARD_E_SERVICE_STOPPED:
				msg += std::string("The Smart card resource manager has shut down.");
				break;

			case SCARD_E_UNSUPPORTED_FEATURE:
				msg += std::string("This smart card does not support the requested feature.");
				break;

#ifdef _WINDOWS
			case SCARD_E_NO_READERS_AVAILABLE:
				msg += std::string("Cannot find a smart card reader.");
				break;

			case SCARD_E_UNEXPECTED:
				msg += std::string("An unexpected card error has occurred.");
				break;

			case SCARD_P_SHUTDOWN:
				msg += std::string("The operation has been aborted to allow the server application to exit.");
				break;

			case SCARD_E_ICC_INSTALLATION:
				msg += std::string("No Primary Provider can be found for the smart card.");
				break;

			case SCARD_E_ICC_CREATEORDER:
				msg += std::string("The requested order of object creation is not supported.");
				break;

			case SCARD_E_DIR_NOT_FOUND:
				msg += std::string("The identified directory does not exist in the smart card.");
				break;

			case SCARD_E_FILE_NOT_FOUND:
				msg += std::string("The identified file does not exist in the smart card.");
				break;

			case SCARD_E_NO_DIR:
				msg += std::string("The supplied path does not represent a smart card directory.");
				break;

			case SCARD_E_NO_FILE:
				msg += std::string("The supplied path does not represent a smart card file.");
				break;

			case SCARD_E_NO_ACCESS:
				msg += std::string("Access is denied to this file.");
				break;

			case SCARD_E_WRITE_TOO_MANY:
				msg += std::string("The smartcard does not have enough memory to store the information.");
				break;

			case SCARD_E_BAD_SEEK:
				msg += std::string("There was an error trying to set the smart card file object pointer.");
				break;

			case SCARD_E_INVALID_CHV:
				msg += std::string("The supplied PIN is incorrect.");
				break;

			case SCARD_E_UNKNOWN_RES_MNG:
				msg += std::string("An unrecognized error code was returned from a layered component.");
				break;

			case SCARD_E_NO_SUCH_CERTIFICATE:
				msg += std::string("The requested certificate does not exist.");
				break;

			case SCARD_E_CERTIFICATE_UNAVAILABLE:
				msg += std::string("The requested certificate could not be obtained.");
				break;

			case SCARD_E_COMM_DATA_LOST:
				msg += std::string("A communications error with the smart card has been detected. Retry the operation.");
				break;

			case SCARD_E_NO_KEY_CONTAINER:
				msg += std::string("The requested key container does not exist on the smart card.");
				break;

			case SCARD_E_SERVER_TOO_BUSY:
				msg += std::string("The Smart card resource manager is too busy to complete this operation.");
				break;

			case SCARD_E_PIN_CACHE_EXPIRED:
				msg += std::string("The smart card PIN cache has expired.");
				break;

			case SCARD_E_NO_PIN_CACHE:
				msg += std::string("The smart card PIN cannot be cached.");
				break;

			case SCARD_E_READ_ONLY_CARD:
				msg += std::string("The smart card is read only and cannot be written to.");
				break;
#endif
			}

			THROW_EXCEPTION_WITH_LOG(CardException, msg);
		}
	}

	void PCSCDataTransport::CheckResult(const void* data, size_t datalen) const
	{
		unsigned char SW1 = reinterpret_cast<const unsigned char*>(data)[datalen - 2];
		unsigned char SW2 = reinterpret_cast<const unsigned char*>(data)[datalen - 1];

		if (datalen >= 2)
		{
			switch (SW1)
			{
			/*case 0x62:
				{
					switch (SW2)
					{
					case 0x82:
						THROW_EXCEPTION_WITH_LOG(CardException, "Command exited before the end");
						break;
					}
				}
				break;

			case 0x63:
				{
					switch (SW2)
					{
					case 0x00:
						THROW_EXCEPTION_WITH_LOG(CardException, "No further information given");
						break;
					}
				}
				break;*/

			case 0x64:
				{
					switch (SW2)
					{
					case 0x00:
						THROW_EXCEPTION_WITH_LOG(CardException, "Execution error");
						break;
					}
				}
				break;

			case 0x65:
				{
					switch (SW2)
					{
					case 0x81:
						THROW_EXCEPTION_WITH_LOG(CardException, "Memory failure");
						break;
					}
				}
				break;

			case 0x67:
				{
					switch (SW2)
					{
					case 0x00:
						THROW_EXCEPTION_WITH_LOG(CardException, "Wrong length");
						break;
					}
				}
				break;

			case 0x68:
				{
					switch (SW2)
					{
					case 0x00:
						THROW_EXCEPTION_WITH_LOG(CardException, "Invalid CLA byte");
						break;
					}
				}
				break;

			case 0x69:
				{
					switch (SW2)
					{
					case 0x81:
						THROW_EXCEPTION_WITH_LOG(CardException, "Command incompatible");
						break;

					case 0x82:
						THROW_EXCEPTION_WITH_LOG(CardException, "Security status not satisfied");
						break;

					case 0x83:
						THROW_EXCEPTION_WITH_LOG(CardException, "Authentication cannot be done");
						break;

					case 0x84:
						THROW_EXCEPTION_WITH_LOG(CardException, "Reference key not useable");
						break;

					case 0x88:
						THROW_EXCEPTION_WITH_LOG(CardException, "Key number not valid");
						break;

					case 0x86:
						THROW_EXCEPTION_WITH_LOG(CardException, "Command not allowed");
						break;
					}
				}
				break;

			case 0x6A:
				{
					switch (SW2)
					{
					case 0x81:
						THROW_EXCEPTION_WITH_LOG(CardException, "Function not supported");
						break;

					case 0x82:
						THROW_EXCEPTION_WITH_LOG(CardException, "File not found or addressed block/byte does not exist");
						break;
					}
				}
				break;

			case 0x6B:
				{
					switch (SW2)
					{
					case 0x00:
						THROW_EXCEPTION_WITH_LOG(CardException, "Wrong parameter P1 or P2");
						break;
					}
				}
				break;

			case 0x91:
				{
					switch (SW2)
					{
					case 0x0E:
						THROW_EXCEPTION_WITH_LOG(CardException, "Insufficient NV-Memory to complete command");
						break;

					case 0x1C:
						THROW_EXCEPTION_WITH_LOG(CardException, "Command code not supported");
						break;

					case 0x1E:
						THROW_EXCEPTION_WITH_LOG(CardException, "CRC or MAC does not match data / Padding bytes not valid");
						break;

					case 0x40:
						THROW_EXCEPTION_WITH_LOG(CardException, "Invalid key number specified");
						break;

					case 0x7E:
						THROW_EXCEPTION_WITH_LOG(CardException, "Length of command string invalid");
						break;

					case 0x9D:
						THROW_EXCEPTION_WITH_LOG(CardException, "Current configuration / status does not allow the requested command");
						break;

					case 0x9E:
						THROW_EXCEPTION_WITH_LOG(CardException, "Value of the parameter(s) invalid");
						break;

					case 0xA0:
						THROW_EXCEPTION_WITH_LOG(CardException, "Requested AID not present on PICC");
						break;

					case 0xAE:
						THROW_EXCEPTION_WITH_LOG(CardException, "Current authentication status does not allow the requested command");
						break;

					case 0xBE:
						THROW_EXCEPTION_WITH_LOG(CardException, "Attempt to read/write data from/to beyond the file's/record's limits");
						break;

					case 0xC1:
						THROW_EXCEPTION_WITH_LOG(CardException, "Unrecoverable error within PICC, PICC will be disabled");
						break;

					case 0xCD:
						THROW_EXCEPTION_WITH_LOG(CardException, "PICC was disabled by an unrecoverable error");
						break;

					case 0xCE:
						THROW_EXCEPTION_WITH_LOG(CardException, "Number of Applications limit reached, no additional CreateApplication possible");
						break;

					case 0xDE:
						THROW_EXCEPTION_WITH_LOG(CardException, "Creation of file/application failed because file/application with same number already exists");
						break;

					case 0xEE:
						THROW_EXCEPTION_WITH_LOG(CardException, "Could not complete NV-write operation due to loss of power, internal backup/rollback mechanism activated");
						break;

					case 0xF0:
						THROW_EXCEPTION_WITH_LOG(CardException, "Specified file number does not exist");
						break;

					case 0xF1:
						THROW_EXCEPTION_WITH_LOG(CardException, "Unrecoverable error within file, file will be disabled");
						break;
					}
				}
				break;
			}
		}
	}
}