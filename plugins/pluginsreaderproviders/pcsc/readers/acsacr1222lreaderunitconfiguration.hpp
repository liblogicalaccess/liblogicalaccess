#pragma once

#include "../pcscreaderunitconfiguration.hpp"

namespace logicalaccess
{
    /**
     * Configuration class for ACSACR1222L.
     */
    class LIBLOGICALACCESS_API ACSACR1222LReaderUnitConfiguration
            : public PCSCReaderUnitConfiguration
    {

    public:

        ACSACR1222LReaderUnitConfiguration();

        virtual void resetConfiguration() override;

        virtual void serialize(boost::property_tree::ptree &parentNode) override;

        virtual void unSerialize(boost::property_tree::ptree &node) override;

        virtual PCSCReaderUnitType getPCSCType() const override;

        const std::string &getUserFeedbackReader() const;

        void setUserFeedbackReader(const std::string &);

    private:
        /**
         * Name of the ReaderUnit that we use as a "feedback" reader.
         *
         * The idea is that the ACSACR1222L as 4 chip: while the main chip
         * will interact with the card, we can connect to one of the 3 chips
         * and use them to drive the LCD display or the LEDs.
         */
        std::string user_feedback_reader_;
    };
}
