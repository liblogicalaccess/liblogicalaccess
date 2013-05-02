/**
 * \file pem.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \brief PEM functions.
 */

#ifndef PEM_HPP
#define PEM_HPP

namespace LOGICALACCESS
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
