/**
 * \file chip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Chip descriptor.
 */

#ifndef LOGICALACCESS_CHIP_HPP
#define LOGICALACCESS_CHIP_HPP

#include <memory>
#include <vector>
#include <logicalaccess/cards/ichip.hpp>
#include <logicalaccess/lla_fwd.hpp>
#include <logicalaccess/lla_core_api.hpp>
#include <logicalaccess/services/cardservice.hpp>

namespace logicalaccess
{
#define CHIP_UNKNOWN "UNKNOWN"
#define CHIP_GENERICTAG "GenericTag"

/**
 * \brief The power status.
 */
typedef enum {
    CPS_NO_POWER   = 0x00, /**< No power */
    CPS_UNKNOWN    = 0x01, /**< Unknown power status */
    CPS_POWER_LOW  = 0x02, /**< Power low */
    CPS_POWER_HIGH = 0x03  /**< Power high */
} ChipPowerStatus;

/**
 * \brief The base chip class for all chip. Each chip have is own object and providers
 * according to himself and the reader used to access the chip.
 */
class LLA_CORE_API Chip : public IChip, public std::enable_shared_from_this<Chip>
{
  public:
    /**
     * \brief Constructor.
     */
    Chip();

    /**
     * \brief Constructor.
     * \param cardtype The Card type of the chip.
     */
    explicit Chip(std::string cardtype);

    /**
     * \brief Destructor.
     */
    virtual ~Chip()
    {
    }

    /**
     * \brief Get the card type of the chip.
     * \return The chip's card type.
     */
    const std::string &getCardType() const override;

    /**
     * \brief Get the generic card type.
     * \return The generic card type.
     */
    std::string getGenericCardType() const override;

    /**
     * \brief Get the root location node.
     * \return The root location node.
     */
    virtual std::shared_ptr<LocationNode> getRootLocationNode();

    /**
     * \brief Get the commands.
     * \return The commands.
     */
    std::shared_ptr<Commands> getCommands() const override
    {
        return d_commands;
    }

    /**
     * \brief Set commands.
     * \param commands The commands.
     */
    void setCommands(std::shared_ptr<Commands> commands)
    {
        d_commands = commands;
    }

    /**
     * \brief Get the chip identifier.
     * \return The chip identifier.
     */
    ByteVector getChipIdentifier() const override
    {
        return d_chipIdentifier;
    }

    /**
     * \brief Set the chip identifier.
     * \param identifier The chip identifier.
     */
    virtual void setChipIdentifier(ByteVector identifier)
    {
        d_chipIdentifier = identifier;
    }

    /**
     * \brief Get the chip power status.
     * \return The power status.
     */
    ChipPowerStatus getPowerStatus() const
    {
        return d_powerStatus;
    }

    /**
     * \brief Set the chip power status.
     * \param powerStatus The power status.
     */
    void setPowerStatus(ChipPowerStatus powerStatus)
    {
        d_powerStatus = powerStatus;
    }

    /**
     * \brief Get the chip reception level.
     * \return The reception level.
     */
    unsigned char getReceptionLevel() const
    {
        return d_receptionLevel;
    }

    /**
     * \brief Set the chip reception level.
     * \param powerStatus The reception level.
     */
    void setReceptionLevel(unsigned char receptionLevel)
    {
        d_receptionLevel = receptionLevel;
    }

    /**
    * \brief Create default access informations.
    * \return Default access informations.
    */
    virtual std::shared_ptr<AccessInfo> createAccessInfo() const;

    /**
    * \brief Create default location.
    * \return Default location.
    */
    virtual std::shared_ptr<Location> createLocation() const;

    /**
     * \brief Get a card service for this chip.
     * \param serviceType The card service type.
     * \return The card service.
     */
    std::shared_ptr<CardService> getService(CardServiceType serviceType) override;

    template<typename T>
    std::shared_ptr<T> getService() {
        return std::dynamic_pointer_cast<T>(getService(T::service_type_));
    }

    bool operator<(const Chip &chip) const
    {
        return d_receptionLevel < chip.getReceptionLevel();
    }

  protected:
    /**
     * \brief The card type of the profile.
     */
    std::string d_cardtype;

    /**
     * \brief The chip identifier.
     */
    ByteVector d_chipIdentifier;

    /**
     * \brief The chip power status. Most chip doesn't have his own power supply.
     */
    ChipPowerStatus d_powerStatus;

    /**
     * \brief Commands object.
     */
    std::shared_ptr<Commands> d_commands;

    /**
     * \brief The chip reception level.
     */
    unsigned char d_receptionLevel;
};
}

#endif /* LOGICALACCESS_CHIP_H */