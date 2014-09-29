/**
 * \file symmetric_key.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Symmetric key base class.
 */

#ifndef SYMETRICKEY_HPP
#define SYMETRICKEY_HPP

#ifdef UNIX
#include <unistd.h>
#endif

#include <vector>

namespace logicalaccess
{
	namespace openssl
	{
		/**
		 * \brief An immutable symmetric key.
		 */
		class SymmetricKey
		{
			public:

				/**
				 * \brief Virtual destructor.
				 *
				 * Required for the class to be polymorphic.
				 */
				virtual inline ~SymmetricKey() {};

				/**
				 * \brief Get the key data.
				 * \return The key data.
				 */
				inline const std::vector<unsigned char>& data() const
				{
					return d_data;
				}

			protected:

				/**
				 * \brief Constructor.
				 * \param size The key size.
				 */
				SymmetricKey(size_t size);

				/**
				 * \brief Constructor.
				 * \param data The buffer data.
				 */
				SymmetricKey(const std::vector<unsigned char>& data);

				/**
				 * \brief Randomize the key.
				 */
				void randomize();

			private:

				/**
				 * \brief The key data.
				 */
				std::vector<unsigned char> d_data;
		};

		/**
		 * \brief Comparaison operator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the keys are equal, false otherwise.
		 */
		inline bool operator==(const SymmetricKey& lhs, const SymmetricKey& rhs)
		{
			return (lhs.data() == rhs.data());
		}

		/**
		 * \brief Comparaison operator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if the keys are different, false otherwise.
		 */
		inline bool operator!=(const SymmetricKey& lhs, const SymmetricKey& rhs)
		{
			return (lhs.data() != rhs.data());
		}

		/**
		 * \brief Comparaison operator.
		 * \param lhs The left argument.
		 * \param rhs The right argument.
		 * \return true if lhs is less than rhs.
		 */
		inline bool operator<(const SymmetricKey& lhs, const SymmetricKey& rhs)
		{
			return (lhs.data() < rhs.data());
		}
	}
}

#endif /* SYMETRICKEY_HPP */

