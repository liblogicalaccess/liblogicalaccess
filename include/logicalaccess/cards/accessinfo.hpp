/**
 * \file accessinfo.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief AccessInfo.
 */

#ifndef LOGICALACCESS_ACCESSINFO_HPP
#define LOGICALACCESS_ACCESSINFO_HPP

#include "logicalaccess/readerproviders/readerprovider.hpp"
#include "logicalaccess/linearizable.hpp"
#include "logicalaccess/key.hpp"

namespace logicalaccess
{
	/**
	 * \brief A Access informations. Describe key used of authentication and access rights for a specific or a group of operati
	 * \remarks Linearizable is depreciated, shouldn't be used anymore in further versions.
	 */
	class LIBLOGICALACCESS_API AccessInfo : public XmlSerializable
	{
		public:

			/**
			 * \brief Constructor.
			 */
			AccessInfo();

			/**
			 * \brief Generate pseudo-random access informations.
			 */
			virtual void generateInfos() = 0;

			/**
			 * \brief Generate pseudo-random access informations for a given card serial number.
			 * \param csn The card serial number, as a salt (no diversification here).
			 */
			virtual void generateInfos(const std::string& csn) = 0;

			/**
			 * \brief Access informations data size.
			 */
			virtual size_t getDataSize() = 0;

			/**
			 * \brief Get the card type for this access infos.
			 * \return The card type.
			 */
			virtual std::string getCardType() const = 0;

			/**
			 * \brief Generate a key data.
			 * \param seed Seed used for random numbers.
			 * \param keySize The key size.
			 * \return A generated pseudo-random key.
			 */
			static std::string generateSimpleKey(long seed, size_t keySize);

			/**
			 * \brief Generate a DES key data.
			 * \param seed Seed used for random numbers.
			 * \param keySize The key size.
			 * \return A generated pseudo-random key.
			 */
			static std::string generateSimpleDESKey(long seed, size_t keySize);

			/**
			 * \brief Equality operator
			 * \param ai Access infos to compare.
			 * \return True if equals, false otherwise.
			 */
			virtual bool operator==(const AccessInfo& ai) const;

			/**
			 * \brief Inequality operator
			 * \param ai Access infos to compare.
			 * \return True if inequals, false otherwise.
			 */
			inline bool operator!=(const AccessInfo& ai) const { return !operator==(ai); };

	};	
}

#endif /* LOGICALACCESS_ACCESSINFO_HPP */
