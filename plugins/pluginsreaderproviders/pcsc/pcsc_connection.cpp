//
// Created by xaqq on 8/3/15.
//

#include <logicalaccess/logs.hpp>
#include <logicalaccess/myexception.hpp>
#include "pcsc_connection.hpp"

logicalaccess::PCSCConnection::PCSCConnection(PCSCShareMode mode,
                                              unsigned long protocol,
                                              SCARDCONTEXT context,
                                              const std::string &device)
    : handle_(0)
    , share_mode_(mode)
    , protocol_(protocol)
	, disposition_(SCARD_LEAVE_CARD)
{
    LOG(DEBUGS) << "Attempting to establish PCSCConnection: Protocol: "
                << pcsc_protocol_to_string(protocol)
                << ", Shared mode: " << pcsc_share_mode_to_string(mode);
    LONG lReturn = SCardConnect(context, device.c_str(), mode, protocol,
                                &handle_, &active_protocol_);
    if (lReturn != SCARD_S_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Failed to establish PCSC connection: " +
                                     PCSCConnection::strerror(lReturn));
    }
    LOG(DEBUGS) << "Established PCSC connection. Active protocol: "
                << pcsc_protocol_to_string(active_protocol_);
}

logicalaccess::PCSCConnection::~PCSCConnection()
{
    if (handle_)
    {
        SCardDisconnect(handle_, disposition_);
    }
}

void logicalaccess::PCSCConnection::reconnect()
{
    LONG lReturn = SCardReconnect(handle_, share_mode_, protocol_,
                                  disposition_, &active_protocol_);
    if (lReturn != SCARD_S_SUCCESS)
    {
        THROW_EXCEPTION_WITH_LOG(LibLogicalAccessException,
                                 "Failed to reconnect PCSC connection: " +
                                     PCSCConnection::strerror(lReturn));
    }
}

std::string logicalaccess::PCSCConnection::strerror(unsigned int error_flag)
{
    std::string msg = std::string("PCSC error: ");

    if (error_flag != SCARD_S_SUCCESS)
    {
        char conv[64];
        sprintf(conv, "%x", error_flag);
        msg += std::string(conv);
        msg += std::string(". ");
        switch ((error_flag | 0x80100000L))
        {
            case SCARD_F_INTERNAL_ERROR:
                msg += "An internal consistency check failed.";
                break;

            case SCARD_E_CANCELLED:
                msg += "The action was cancelled by a Cancel request.";
                break;

            case SCARD_E_INVALID_HANDLE:
                msg += "The supplied handle was invalid.";
                break;

            case SCARD_E_INVALID_PARAMETER:
                msg += "One or more of the supplied parameters "
                       "could not be properly interpreted.";
                break;

            case SCARD_E_INVALID_TARGET:
                msg += "Registry startup information is missing or invalid.";
                break;

            case SCARD_E_NO_MEMORY:
                msg += "Not enough memory available to complete this command.";
                break;

            case SCARD_F_WAITED_TOO_LONG:
                msg += "An internal consistency timer has expired.";
                break;

            case SCARD_E_INSUFFICIENT_BUFFER:
                msg += "The data buffer to receive returned data "
                       "is too small for the returned data.";
                break;

            case SCARD_E_UNKNOWN_READER:
                msg += "The specified reader name is not recognized.";
                break;

            case SCARD_E_TIMEOUT:
                msg += "The user-specified timeout value has expired.";
                break;

            case SCARD_E_SHARING_VIOLATION:
                msg += "The smart card cannot be accessed because "
                       "of other connections outstanding.";
                break;

            case SCARD_E_NO_SMARTCARD:
                msg += "The operation requires a Smart Card, but "
                       "no Smart Card is currently in the device.";
                break;

            case SCARD_E_UNKNOWN_CARD:
                msg += "The specified smart card name is not recognized.";
                break;

            case SCARD_E_CANT_DISPOSE:
                msg += "The system could not dispose of the media "
                       "in the requested manner.";
                break;

            case SCARD_E_PROTO_MISMATCH:
                msg += "The requested protocols are incompatible "
                       "with the protocol currently in use with "
                       "the smart card.";
                break;

            case SCARD_E_INVALID_VALUE:
                msg += "One or more of the supplied parameters "
                       "values could not be properly interpreted.";
                break;

            case SCARD_E_SYSTEM_CANCELLED:
                msg += "The action was cancelled by the system, "
                       "presumably to log off or shut down.";
                break;

            case SCARD_F_COMM_ERROR:
                msg += "An internal communications error has been detected.";
                break;

            case SCARD_F_UNKNOWN_ERROR:
                msg += "An internal error has been detected, but "
                       "the source is unknown.";
                break;

            case SCARD_E_INVALID_ATR:
                msg += "An ATR obtained from the registry is not a "
                       "valid ATR string.";
                break;

            case SCARD_E_NOT_TRANSACTED:
                msg += "An attempt was made to end a non-existent transaction.";
                break;

            case SCARD_E_READER_UNAVAILABLE:
                msg +=
                    "The specified reader is not currently available for use.";
                break;

            case SCARD_E_PCI_TOO_SMALL:
                msg += "The PCI Receive buffer was too small.";
                break;

            case SCARD_E_READER_UNSUPPORTED:
                msg += "The reader driver does not meet minimal "
                       "requirements for support.";
                break;

            case SCARD_E_DUPLICATE_READER:
                msg +=
                    "The reader driver did not produce a unique reader name.";
                break;

            case SCARD_E_CARD_UNSUPPORTED:
                msg += "The smart card does not meet minimal "
                       "requirements for support.";
                break;

            case SCARD_E_NO_SERVICE:
                msg += "The Smart card resource manager is not running.";
                break;

            case SCARD_E_SERVICE_STOPPED:
                msg += "The Smart card resource manager has shut down.";
                break;

            case SCARD_E_UNSUPPORTED_FEATURE:
                msg +=
                    "This smart card does not support the requested feature.";
                break;

#ifdef _WINDOWS
            case SCARD_E_NO_READERS_AVAILABLE:
                msg += "Cannot find a smart card reader.";
                break;

            case SCARD_E_UNEXPECTED:
                msg += "An unexpected card error has occurred.";
                break;

            case SCARD_P_SHUTDOWN:
                msg += "The operation has been aborted to allow "
                       "the server application to exit.";
                break;

            case SCARD_E_ICC_INSTALLATION:
                msg += "No Primary Provider can be found for the smart card.";
                break;

            case SCARD_E_ICC_CREATEORDER:
                msg +=
                    "The requested order of object creation is not supported.";
                break;

            case SCARD_E_DIR_NOT_FOUND:
                msg += "The identified directory does not exist in "
                       "the smart card.";
                break;

            case SCARD_E_FILE_NOT_FOUND:
                msg += "The identified file does not exist in the smart card.";
                break;

            case SCARD_E_NO_DIR:
                msg += "The supplied path does not represent a "
                       "smart card directory.";
                break;

            case SCARD_E_NO_FILE:
                msg +=
                    "The supplied path does not represent a smart card file.";
                break;

            case SCARD_E_NO_ACCESS:
                msg += "Access is denied to this file.";
                break;

            case SCARD_E_WRITE_TOO_MANY:
                msg += "The smartcard does not have enough memory "
                       "to store the information.";
                break;

            case SCARD_E_BAD_SEEK:
                msg += "There was an error trying to set the smart "
                       "card file object pointer.";
                break;

            case SCARD_E_INVALID_CHV:
                msg += "The supplied PIN is incorrect.";
                break;

            case SCARD_E_UNKNOWN_RES_MNG:
                msg += "An unrecognized error code was returned "
                       "from a layered component.";
                break;

            case SCARD_E_NO_SUCH_CERTIFICATE:
                msg += "The requested certificate does not exist.";
                break;

            case SCARD_E_CERTIFICATE_UNAVAILABLE:
                msg += "The requested certificate could not be "
                       "obtained.";
                break;

            case SCARD_E_COMM_DATA_LOST:
                msg += "A communications error with the smart card "
                       "has been detected. Retry the operation.";
                break;

            case SCARD_E_NO_KEY_CONTAINER:
                msg += "The requested key container does not exist "
                       "on the smart card.";
                break;

            case SCARD_E_SERVER_TOO_BUSY:
                msg += "The Smart card resource manager is too "
                       "busy to complete this operation.";
                break;

            case SCARD_E_PIN_CACHE_EXPIRED:
                msg += "The smart card PIN cache has expired.";
                break;

            case SCARD_E_NO_PIN_CACHE:
                msg += "The smart card PIN cannot be cached.";
                break;

            case SCARD_E_READ_ONLY_CARD:
                msg += "The smart card is read only and cannot be written to.";
                break;

            case SCARD_W_REMOVED_CARD:
                msg += "The smart card has been removed, so further "
                       "communication is not possible.";
                break;
#endif
        }
    }
    return msg;
}

SCARDHANDLE logicalaccess::PCSCConnection::getHandle() const
{
    return handle_;
}
