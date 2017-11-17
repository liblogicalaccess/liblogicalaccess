/**
 * \file accesscontrolcardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Access Control Card service.
 */

#ifndef LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP
#define LOGICALACCESS_ACCESSCONTROLCARDSERVICE_HPP

#include <logicalaccess/services/cardservice.hpp>
#include <logicalaccess/services/accesscontrol/formats/format.hpp>
#include <logicalaccess/cards/location.hpp>
#include <logicalaccess/cards/accessinfo.hpp>

namespace logicalaccess
{
/**
* \brief A format list.
*/
typedef std::vector<std::shared_ptr<Format>> FormatList;

#define ACCESSCONTROLCARDSERVICE "AccessControl"

/**
 * \brief The base access control card service class for all access control services.
 */
class LIBLOGICALACCESS_API AccessControlCardService : public CardService
{
  public:
    /**
     * \brief Constructor.
     * \param chip The associated chip.
     */
    explicit AccessControlCardService(std::shared_ptr<Chip> chip);

    /**
     * \brief Destructor.
     */
    virtual ~AccessControlCardService();

    std::string getCSType() override { return ACCESSCONTROLCARDSERVICE; }

    /**
     * \brief Read format from the card.
     * \param format The format to read.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \return The format read on success, null otherwise.
     */
    virtual std::shared_ptr<Format> readFormat(std::shared_ptr<Format> format,
                                               std::shared_ptr<Location> location,
                                               std::shared_ptr<AccessInfo> aiToUse);

    /**
     * \brief Write format to the card.
     * \param format The format to write.
     * \param location The format location.
     * \param aiToUse The key's informations to use.
     * \param aiToWrite The key's informations to write.
     * \return True on success, false otherwise.
     */
    virtual bool writeFormat(std::shared_ptr<Format> format,
                             std::shared_ptr<Location> location,
                             std::shared_ptr<AccessInfo> aiToUse,
                             std::shared_ptr<AccessInfo> aiToWrite);

    /**
    * \brief Get the supported format list.
    * \return The format list.
    */
    virtual FormatList getSupportedFormatList();

    /**
    * \brief Get the HID Wiegand format list.
    * \return The format list.
    */
    FormatList getHIDWiegandFormatList() const;
};
}

#endif