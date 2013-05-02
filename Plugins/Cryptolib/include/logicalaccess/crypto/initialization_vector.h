/**
 * \file initialization_vector.hpp
 * \author Julien KAUFFMANN <julien.kauffmann@islog.eu>
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Initialization vector base class.
 */

#ifndef INITIALIZATION_VECTOR_HPP
#define INITIALIZATION_VECTOR_HPP

#include <vector>
#include <iostream>

namespace LOGICALACCESS
{
	namespace openssl
	{
		/**
		 * \brief An initialisation vector.
		 *
		 * InitializationVector serves as a base class for AESInitializationVector.
		 */
		class InitializationVector
		{
			public:

				/**
				 * \brief Get the IV data.
				 * \return The IV data.
				 */
				inline const std::vector<unsigned char>& data() const
				{
					return d_data;
				}

			protected:

				/**
				 * \brief Create a new InitializationVector.
				 * \param size The IV size.
				 * \param random true if the IV must be randomized, false otherwise.
				 */
				InitializationVector(size_t size, bool random);

				/**
				 * \brief Create an InitializationVector from existing data.
				 * \param data The data.
				 */
				InitializationVector(const std::vector<unsigned char>& data);

				/**
				 * \brief Zero the IV.
				 */
				void zero();

				/**
				 * \brief Randomize the IV.
				 */
				void randomize();

			private:

				/**
				 * \brief The IV data.
				 */
				std::vector<unsigned char> d_data;
		};
	}
}

#endif /* INITIALIZATION_VECTOR_HPP */

