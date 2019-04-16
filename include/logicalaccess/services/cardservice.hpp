/**
 * \file cardservice.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Card service.
 */

#ifndef LOGICALACCESS_CARDSERVICE_HPP
#define LOGICALACCESS_CARDSERVICE_HPP

#include <memory>
#include <string>
#include <logicalaccess/lla_core_api.hpp>

namespace logicalaccess
{
#define EXCEPTION_MSG_AUTHENTICATE                                                       \
    "Authenticate failed." /**< \brief The authentication exception message */
#define EXCEPTION_MSG_LOADKEY                                                            \
    "Load key failed." /**< \brief The load key exception message */
#define EXCEPTION_MSG_NOKEY                                                              \
    "No key found." /**< \brief The no key found exception message */
#define EXCEPTION_MSG_READ                                                               \
    "Read operation failed." /**< \brief The read exception message */
#define EXCEPTION_MSG_WRITE                                                              \
    "Write operation failed." /**< \brief The write exception message */
#define EXCEPTION_MSG_CHANGEKEY                                                          \
    "Change key failed." /**< \brief The change key exception message */

class Chip;

/**
 * \brief The card services.
 */
typedef enum
{
    CST_UNDEFINED      = 0x0000, /**< Undefined card service type */
    CST_ACCESS_CONTROL = 0x0001, /**< The access control card service type */
    CST_NFC_TAG        = 0x0002, /**< The NFC Tag card service type */
    CST_STORAGE        = 0x0003, /**< The storage card service type */
    CST_UID_CHANGER    = 0x0004, /**< Service to change the UID of a card */
    CST_IDENTITY       = 0x0005, /**< Service to query identity information (EPassport) */
    CST_JSON_DUMP      = 0x0006, /**< Service to dump a card content in JSON */
} CardServiceType;

/**
 * \brief The base card service class for all services.
 */
class LLA_CORE_API CardService : public std::enable_shared_from_this<CardService>
{
  public:
    virtual ~CardService() = default;

    /**
     * \brief Constructor.
     * \param chip The associated chip.
     * \param serviceType The card service type.
     */
    CardService(std::shared_ptr<Chip> chip, CardServiceType serviceType);

    virtual std::string getCSType() = 0;

    /*
     * \brief Get the associated chip object.
     * \return The chip.
     */
    std::shared_ptr<Chip> getChip() const
    {
        return d_chip;
    }

    /**
     * \brief Get the card service type.
     * \return The card service type.
     */
    virtual CardServiceType getServiceType() const;

  protected:
    /**
     * \brief Chip object.
     */
    std::shared_ptr<Chip> d_chip;

    /**
     * \brief Card service type.
     */
    CardServiceType d_serviceType;
};
}

#endif