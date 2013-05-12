/**
 * \file openssl.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief OpenSSL global functions.
 */

#ifndef OPENSSL_HPP
#define OPENSSL_HPP

#include <boost/noncopyable.hpp>

namespace logicalaccess
{
	namespace openssl
	{
		/**
		 * \brief Initialize OpenSSL library.
		 * \warning MUST be called in the main thread, at the very beginning of the application.
		 */
		void initialize();

		/**
		 * \brief Clean up OpenSSL library.
		 * \warning MUST be called in the main thread, at the very end of the application.
		 */
		void cleanup();

		class Initializer : public boost::noncopyable
		{
			public:

				/**
				 * \brief Create the initializer and call initialize.
				 */
				Initializer()
				{
					initialize();
				}

				/**
				 * \brief Destroy the initializer and call cleanup.
				 */
				~Initializer()
				{
					cleanup();
				}
		};
	}
}

#endif /* OPENSSL_HPP */

