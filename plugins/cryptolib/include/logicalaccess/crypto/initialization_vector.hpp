/**
 * \file initialization_vector.hpp
 * \author Julien K. <julien-dev@islog.com>
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Initialization vector base class.
 */

#ifndef INITIALIZATION_VECTOR_HPP
#define INITIALIZATION_VECTOR_HPP

#include <vector>
#include <iostream>
#include <random>

namespace logicalaccess
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

				/**
				 * \brief mt19937 is a standard mersenne_twister_engine
				 */
				std::mt19937 m_rand;
		};
	}
}

#endif /* INITIALIZATION_VECTOR_HPP */

