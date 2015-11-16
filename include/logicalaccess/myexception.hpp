#ifndef __MYEXCEPTION_HPP__
#define __MYEXCEPTION_HPP__

#include <string>
#include <exception>
#include <functional>

#if defined(UNIX) || _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#include "logicalaccess_api.hpp"

namespace logicalaccess
{
    namespace Exception
    {
        class exception : public std::exception
        {
        public:
            exception(const std::string& msg) : _message(msg) {};

            const char* what() const NOEXCEPT
            {
                return this->_message.c_str();
            }
        private:
            std::string _message;
        };

        class bad_function_call : public std::bad_function_call
        {
        public:
            bad_function_call(const std::string& msg) : _message(msg) {};

            const char* what() const NOEXCEPT
            {
                return this->_message.c_str();
            }
        private:
            std::string _message;
        };
    }

    /**
       * \brief A liblogicalaccess exception class.
       */
    class LIBLOGICALACCESS_API LibLogicalAccessException : public Exception::exception
    {
    public:
        LibLogicalAccessException(const std::string& message)
            : exception(message)
        {};
    };

    /**
     * Exception class to notify of error on card operation.
	 *
	 * If a command fails and the error is known, a CardException corresponding
	 * to the error will be thrown by the library.
     */
    class LIBLOGICALACCESS_API CardException : public LibLogicalAccessException
    {
    public:
        CardException(const std::string& message)
            : LibLogicalAccessException(message),
			error_type_(DEFAULT)
        {
		};

		/**
		* This is a general enumeration to describe possible error.
		* This is usefull because not all readers / card combination fail with the
		* same error code for the same error.
		*/
		enum ErrorType
		{
			DEFAULT, // Either the exception was constructed with the legacy constructor and 
			         // did not specify the error code, or the exception is too specific to
					 // be listed here. In the later case, rely on the error message for more
					 // information.
			EXECUTION_ERROR,
			MEMORY_FAILURE,
			WRONG_LENGTH,
			INVALID_CLA_BYTE,
			COMMAND_INCOMPATIBLE,
			SECURITY_STATUS,
			AUTHENTICATION_FAILURE,
			REFERENCE_KEY_UNUSABLE,
			COMMAND_NOT_ALLOWED,
			KEY_NUMBER_INVALID,
			FUNCTION_NOT_SUPPORTED,
			NOT_FOUND,  // File not found or block/byte does not exist.
			WRONG_P1_P2,
			WRONG_LE,
			WRONG_INSTRUCTION,
			WRONG_CLASS,
			UNKOWN_ERROR // Operation failed, no precise diagnosis
		};

		CardException(const std::string &message, ErrorType error) :
			LibLogicalAccessException(message),
			error_type_(error)
		{
		}			

		enum ErrorType error_code() const
		{
			return error_type_;
		}

	protected:
		enum ErrorType error_type_;

    };

#define EXCEPTION_MSG_CONNECTED			"Already connected to a card. Please disconnect before." /**< \brief Not connected exception message */
#define EXCEPTION_MSG_NOREADER			"No reader found." /**< \brief No reader found exception format */
#define EXCEPTION_MSG_LICENSENOACCESS	"The current license doesn't allow this functionality." /**< \brief License no access exception message  */
}

#endif