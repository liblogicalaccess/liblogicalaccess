/**
 * \file location.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Location.
 */

#ifndef LOGICALACCESS_LOCATION_HPP
#define LOGICALACCESS_LOCATION_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/linearizable.hpp"


namespace logicalaccess
{
	/**
	 * \brief A location informations.
	 */
	class LIBLOGICALACCESS_API Location : public Linearizable, public XmlSerializable
	{
	public:
			/**
			 * \brief Location informations data size.
			 */
			virtual size_t getDataSize() = 0;

			/**
			 * \brief Get the card type for this location.
			 * \return The card type.
			 */
			virtual std::string getCardType() = 0;

			/**
			 * \brief Equality operator
			 * \param location Location to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const Location& location) const;

			/**
			 * \brief Inequality operator
			 * \param location Location to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const Location& location) const { return !operator==(location); };
	};	
}

#endif /* LOGICALACCESS_LOCATION_HPP */
