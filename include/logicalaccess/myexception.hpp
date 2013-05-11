#ifndef __MYEXCEPTION_HPP__
#define __MYEXCEPTION_HPP__

#include <string>
#include <exception>
#include <functional>

#ifdef UNIX
#define NOEXCEPT noexcept
#else
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
}

#endif
