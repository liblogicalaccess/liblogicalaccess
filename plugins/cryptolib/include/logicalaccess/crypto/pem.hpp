/**
 * \file pem.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief PEM functions.
 */

#ifndef PEM_HPP
#define PEM_HPP

namespace logicalaccess
{
    namespace openssl
    {
        /**
         * \brief Default PEM passphrase callback type.
         */
        typedef int PEMPassphraseCallback(char*, int, int, void*);
    }
}

#endif /* PEM_HPP */