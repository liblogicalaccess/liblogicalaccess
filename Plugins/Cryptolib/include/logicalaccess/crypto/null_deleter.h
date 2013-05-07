/**
 * \file null_deleter.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \brief A null deleter class.
 */

#ifndef NULL_DELETER_HPP
#define NULL_DELETER_HPP

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief A null deleter structure.
		 *
		 * Sometimes, you want to use a boost::shared_ptr to reference a staticaly allocated object that must *NOT* be deallocated when the shared_ptr is destroyed. null_deleter serves this purpose.
		 */
		struct null_deleter
		{
			/**
			 * \brief The null deletion function.
			 */
			void operator() (void const*) const {};
		};
	}
}

#endif /* NULL_DELETER_HPP */

