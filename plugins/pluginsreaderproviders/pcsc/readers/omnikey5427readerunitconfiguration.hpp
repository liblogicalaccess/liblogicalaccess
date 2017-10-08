//
// Created by xaqq on 4/13/15.
//

#pragma once

#include <logicalaccess/cards/aes128key.hpp>
#include "../pcscreaderunitconfiguration.hpp"

namespace logicalaccess
{
    /**
     * The Omnikey 5427 reader unit configuration class.
     */
    class LIBLOGICALACCESS_API Omnikey5427ReaderUnitConfiguration
            : public PCSCReaderUnitConfiguration
    {
    public:
        Omnikey5427ReaderUnitConfiguration();

        /**
         * Are we using secure mode?
         * \return The secure mode value, true if used, false otherwise.
         */
        bool getUseSecureMode() const;

        /**
         * Enabled or disable secure mode use.
         * \param use_sm The secure mode value, true if used, false otherwise.
         */
        void setUseSecureMode(bool use_sm);


        /**
         * Return the 128 bits AES key used to establish
         * a secure session with the reader.
         */
        std::shared_ptr<AES128Key> getSecureMasterKey() const;

        /**
         * Set a 128 bits AES key that will be used as
         * the master key when enabling a secure session
         * with the reader.
         */
        void setSecureMasterKey( std::shared_ptr<AES128Key>);

	    void resetConfiguration() override;

	    void serialize(boost::property_tree::ptree &parentNode) override;

	    void unSerialize(boost::property_tree::ptree &node) override;

	    PCSCReaderUnitType getPCSCType() const override;

    private:
        bool useSecureMode_;
        std::shared_ptr<AES128Key> masterKey_;
    };

}