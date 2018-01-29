#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>

#ifndef BOOST_VERSION_TYPES_HPP
#define BOOST_VERSION_TYPES_HPP

#if BOOST_VERSION >= 106600
#define BOOST_ASIO_MAKE_ADDRESS boost::asio::ip::make_address
#else
#define BOOST_ASIO_MAKE_ADDRESS boost::asio::ip::address::from_string
#endif

#endif