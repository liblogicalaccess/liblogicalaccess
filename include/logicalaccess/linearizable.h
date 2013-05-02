/**
 * \file linearizable.h
 * \author Julien Kauffmann <julien.kauffmann@islog.eu>
 * \brief Linearizable base class.
 */

#ifndef LINEARIZABLE_H
#define LINEARIZABLE_H

#include "logicalaccess/BufferHelper.h"

namespace LOGICALACCESS
{
	/**
	 * \brief A linearizable base class. Add binary serialization to a class.
	 */
	class LIBLOGICALACCESS_API Linearizable
	{
		public:

			/**
			 * \brief Destructor.
			 */
			virtual ~Linearizable();

			/**
			 * \brief Export location informations to a buffer.
			 * \param data The buffer.
			 */
			virtual std::vector<unsigned char> getLinearData() const = 0;

			/**
			 * \brief Import location informations from a buffer.
			 * \param data The buffer.
			 * \param offset The offset.
			 */
			virtual void setLinearData(const std::vector<unsigned char>& data, size_t offset) = 0;
	};	
}

#endif /* LINEARIZABLE_H */
