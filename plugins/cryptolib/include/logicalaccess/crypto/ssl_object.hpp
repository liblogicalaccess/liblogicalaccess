/**
 * \file ssl_object.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A ssl object class.
 */

#ifndef SSL_OBJECT_HPP
#define SSL_OBJECT_HPP

#include <string>

#include <memory>

#include <openssl/ssl.h>

namespace logicalaccess
{
    namespace openssl
    {
        class SSLContext;

        /**
         * \brief A SSL object.
         */
        class SSLObject
        {
        public:

            /**
             * \brief Get the associated data.
             * \return The associated data.
             */
            void* getAppData() const;

            /**
             * \brief Set the associated data.
             * \param data The data.
             */
            void setAppData(void* data) const;

            /**
             * \brief Get the associated data.
             * \return The associated data.
             */
            template <typename T> T* getAppData()
            {
                return reinterpret_cast<T*>(getAppData());
            }

            /**
             * \brief Get the associated SSL context.
             * \return The associated SSL context.
             */
            SSLContext sslContext() const;

            /**
             * \brief Get the raw pointer.
             * \return The raw pointer.
             */
	        std::shared_ptr<SSL> getRaw() const
	        {
                return d_ssl;
            }

        private:

            /**
             * \brief Constructor.
             * \param ssl The SSL object to use. If ssl is NULL, the behavior is undefined.
             */
	        explicit SSLObject(std::shared_ptr<SSL> ssl);

            /**
             * \brief The internal pointer.
             */
            std::shared_ptr<SSL> d_ssl;

            friend class X509CertificateStoreContext;
        };
    }
}

#endif /* SSL_OBJECT_HPP */