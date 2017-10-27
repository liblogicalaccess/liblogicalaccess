#pragma once

namespace logicalaccess
{
/**
* A collection of function to colorize / format text.
*/
namespace Colorize
{
/**
* Implementation namespace.
*
* Do not use.
*/
namespace detail
{
static std::string clear()
{
    return "\033[0m";
}

/**
* Return a string containing the escape code, a string representation
* of T and the clear escape string.
*/
template <typename T>
std::string format(const std::string &escape_code, const T &in)
{
    std::stringstream ss;
    ss << "\033[" << escape_code << "m" << in << clear();
    return ss.str();
}
}

template <typename T>
std::string bold(const T &in)
{
    return detail::format("1", in);
}

template <typename T>
std::string underline(const T &in)
{
    return detail::format("4", in);
}

template <typename T>
std::string red(const T &in)
{
    return detail::format("31", in);
}

template <typename T>
std::string yellow(const T &in)
{
    return detail::format("33", in);
}

template <typename T>
std::string green(const T &in)
{
    return detail::format("32", in);
}
}
}
