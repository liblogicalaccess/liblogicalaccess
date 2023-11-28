/**
 * \file wiegandformat.hpp
 * \author Arnaud H <arnaud-dev@islog.com>, Maxime C. <maxime@leosac.com>
 * \brief Wiegand format.
 */

#ifndef LOGICALACCESS_WIEGANDFORMAT_HPP
#define LOGICALACCESS_WIEGANDFORMAT_HPP

#include <logicalaccess/services/accesscontrol/formats/staticformat.hpp>

namespace logicalaccess
{
/**
 * \brief A Wiegand format.
 */
class LLA_CORE_API WiegandFormat : public StaticFormat
{
  public:
    /**
     * \brief Constructor.
     *
     * Create a Wiegand Format.
     */
    WiegandFormat();

    /**
     * \brief Destructor.
     *
     * Release the Wiegand Format.
     */
    virtual ~WiegandFormat();

    /**
     * \brief Get left parity byte.
     * \return The left parity.
     */
    unsigned char getLeftParity() const;

    /**
     * \brief Get right parity byte.
     * \return The right parity.
     */
    unsigned char getRightParity() const;

    /**
     * \brief Get linear data.
     * \param data Where to put data
     * \param dataLengthBytes Length in byte of data
     */
    ByteVector getLinearData() const override;

    /**
     * \brief Get linear data without parity
     * \param data Buffer to be modified. Should be allocated.
     * \param dataLengthBytes Length in byte of data
     */
    virtual BitsetStream getLinearDataWithoutParity() const = 0;

    /**
     * \brief Set linear data
     * \param data Where to get data
     * \param dataLengthBytes Length of data in bytes
     */
    void setLinearData(const ByteVector &data) override;

    /**
     * \brief Write linear data without parity into data buffer
     * \param data Buffer where data will be written.
     * \param dataLengthBytes Length of data in bytes
     */
    virtual void setLinearDataWithoutParity(const ByteVector &data) = 0;

  protected:
    /**
     * \brief The left parity length.
     */
    unsigned int d_leftParityLength;

    /**
     * \brief The left parity type.
     */
    ParityType d_leftParityType;

    /**
     * \brief The right parity length.
     */
    unsigned int d_rightParityLength;

    /**
     * \brief The right parity type.
     */
    ParityType d_rightParityType;
};
}

#endif /* LOGICALACCESS_WIEGANDFORMAT_HPP */