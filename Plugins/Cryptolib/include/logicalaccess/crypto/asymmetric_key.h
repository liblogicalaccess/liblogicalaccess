/**
 * \file asymmetric_key.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@isog.eu>
 * \brief The base class for asymmetric keys.
 */

#ifndef ASYMETRICKEY_HPP
#define ASYMETRICKEY_HPP

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief The asymmetric keys base class.
		 *
		 * This class just serves as a base class for all asymmetric key classes. It cannot be instanciated.
		 */
		class AsymmetricKey
		{
			public:

				/**
				 * \brief Virtual destructor.
				 *
				 * Required for the class to be polymorphic.
				 */
				virtual ~AsymmetricKey() {};

			protected:

				/**
				 * \brief Constructor.
				 *
				 * A protected constructor, just to prevent instanciation.
				 */
				inline AsymmetricKey() {};
		};
	}
}

#endif /* ASYMETRICKEY_HPP */

