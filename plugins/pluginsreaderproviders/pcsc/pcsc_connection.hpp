//
// Created by xaqq on 8/3/15.
//

#ifndef LIBLOGICALACCESS_PCSC_CONNECTION_HPP
#define LIBLOGICALACCESS_PCSC_CONNECTION_HPP

#include "pcscreaderunitconfiguration.hpp"

namespace logicalaccess
{
    /**
     * Abstract away a PCSC connection.
     *
     */
    class PCSCConnection
    {
    public:
        /**
         * Establish the PCSC connection
         */
        PCSCConnection(PCSCShareMode mode,
                       unsigned long protocol,
                       SCARDCONTEXT context,
                       const std::string &device);

        /**
         * Disconnects the PCSC connection
         */
        ~PCSCConnection();

		/**
		 * Check for a PCSC error in error_flag.
		 *
		 * Return a string corresponding to the error, or an empty
		 * string if no error.
		 */
		static std::string strerror(unsigned int error_flag);

        /**
         * The handle.
         */
        SCARDHANDLE handle_;

        /**
         * The share mode.
         */
        PCSCShareMode share_mode_;

        /**
         * The activated protocol.
         */
        DWORD protocol_;
    };
}

#endif //LIBLOGICALACCESS_PCSC_CONNECTION_HPP
