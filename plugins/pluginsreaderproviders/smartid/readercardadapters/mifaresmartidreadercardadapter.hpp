/**
 * \file mifaresmartidreadercardadapter.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SmartID reader/card adapter.
 */

#ifndef LOGICALACCESS_MIFARESMARTIDREADERCARDADAPTER_HPP
#define LOGICALACCESS_MIFARESMARTIDREADERCARDADAPTER_HPP

#include "smartidreadercardadapter.hpp"

#include <string>
#include <vector>

#include "logicalaccess/logs.hpp"

namespace logicalaccess
{
    /**
     * \brief A Mifare SmartID reader/card adapter class.
     */
    class LIBLOGICALACCESS_API MifareSmartIDReaderCardAdapter : public SmartIDReaderCardAdapter
    {
    public:

        /**
         * \brief Send a command to the reader.
         * \param cmd The command code.
         * \param command The command buffer.
         * \param timeout The command timeout.
         * \return The result of the command.
         */
		ByteVector sendCommand(unsigned char cmd, const ByteVector& command, long int timeout = 1000) override;

        /**
         * \brief Send a REQ command from the PCD to the PICC.
         * \return The ATQ value.
         */
		ByteVector request() override;

        /**
         * \brief Manage collision.
         * \return PICC serial number.
         */
		ByteVector anticollision() override;

        /**
         * \brief Select a PICC.
         * \param uid The PICC serial number.
         */
	    void selectIso(const ByteVector& uid) override;

        /**
         * \brief Select a PICC.
         * \param uid The PICC serial number.
         */
        virtual void select(const ByteVector& uid);

        /**
         * \brief Send a HLT command from the PCD to the PICC.
         */
	    void halt() override;
    };
}

#endif /* LOGICALACCESS_MIFARESMARTIDREADERCARDADAPTER_HPP */