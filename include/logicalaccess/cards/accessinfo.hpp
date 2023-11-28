/**
 * \file accessinfo.hpp
 * \author Maxime C. <maxime@leosac.com>
 * \brief AccessInfo.
 */

#ifndef LOGICALACCESS_ACCESSINFO_HPP
#define LOGICALACCESS_ACCESSINFO_HPP

#include <logicalaccess/readerproviders/readerprovider.hpp>
#include <logicalaccess/linearizable.hpp>
#include <logicalaccess/key.hpp>

namespace logicalaccess
{
/**
 * \brief A Access informations. Describe key used of authentication and access rights for
 * a specific or a group of operati
 * \remarks Linearizable is depreciated, shouldn't be used anymore in further versions.
 */
class LLA_CORE_API AccessInfo : public XmlSerializable
{
  public:
    /**
     * \brief Constructor.
     */
    AccessInfo();

    virtual ~AccessInfo() = default;

    /**
     * \brief Generate pseudo-random access informations.
     */
    virtual void generateInfos() = 0;

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
    static std::string generateSimpleKey(size_t keySize);

    /**
     * \brief Generate a DES key data.
     * \param seed Seed used for random numbers.
     * \param keySize The key size.
     * \return A generated pseudo-random key.
     */
    static std::string generateSimpleDESKey(size_t keySize);

    /**
     * \brief Equality operator
     * \param ai Access infos to compare.
     * \return True if equals, false otherwise.
     */
    virtual bool operator==(const AccessInfo &ai) const;

    /**
     * \brief Inequality operator
     * \param ai Access infos to compare.
     * \return True if inequals, false otherwise.
     */
    bool operator!=(const AccessInfo &ai) const
    {
        return !operator==(ai);
    }
};
}

#endif /* LOGICALACCESS_ACCESSINFO_HPP */