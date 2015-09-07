//
// Created by xaqq on 7/31/15.
//

#ifndef LIBLOGICALACCESS_PCSC_CTL_DATATRANSPORT_HPP
#define LIBLOGICALACCESS_PCSC_CTL_DATATRANSPORT_HPP


#include "pcscdatatransport.hpp"

namespace logicalaccess
{

    /**
     * A PCSC Datatransport that transmits command using the
     * SCardControl() function instead of the SCardTransmit().
     *
     * This data transport is useful when talking to the reader in direct mode.
     */
    class PCSCControlDataTransport : public PCSCDataTransport
    {

    public:
        virtual void send(const std::vector<unsigned char> &data) override;
    };

}


#endif //LIBLOGICALACCESS_PCSC_CTL_DATATRANSPORT_HPP
