#ifndef __MYEXCEPTION_HPP__
#define __MYEXCEPTION_HPP__

#include <string>
#include <exception>
#include <functional>

#ifdef UNIX
#define NOEXCEPT noexcept
#ifndef LIBLOGICALACCESS_API
#define LIBLOGICALACCESS_API
#endif
#else
#include "logicalaccess/msliblogicalaccess.h"
#define NOEXCEPT
#endif

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
	 * \brief A card exception class.
	 */
	class LIBLOGICALACCESS_API CardException : public LibLogicalAccessException
	{
	public:
		CardException(const std::string& message)
			: LibLogicalAccessException(message)
		{};
	};

	#define EXCEPTION_MSG_CONNECTED			"Already connected to a card. Please disconnect before." /**< \brief Not connected exception message */
	#define EXCEPTION_MSG_NOREADER			"No reader found." /**< \brief No reader found exception format */
	#define EXCEPTION_MSG_LICENSENOACCESS	"The current license doesn't allow this functionality." /**< \brief License no access exception message  */
}

#endif
