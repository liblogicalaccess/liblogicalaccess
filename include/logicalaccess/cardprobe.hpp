#pragma once

#include "logicalaccess/lla_fwd.hpp"
#include <string>

namespace logicalaccess
{

/**
 * An object whose responsibility is to detect the type of card
 * by sending command to it and analysing response.
 *
 * The probe makes use of a ReaderUnit to create temporary chips and
 * send them commands.
 *
 * The probing infrastructure is not very robust because hardware behaviour
 * if not properly defined. This class may be reimplemented for some
 * reader unit that behave differently.
 */
class CardProbe
{
      public:
        CardProbe(ReaderUnit *ru);
        CardProbe(const CardProbe &) = delete;

        virtual std::string guessCardType();

      public:
        /**
         * Test if the card is DESFire by sending the getversion command.
         *
         * The optional `*uid` vector will be populate with the uid retrieve from
         * the getversion command, unless the command failed.
         */
        virtual bool is_desfire(std::vector<uint8_t> *uid = nullptr) = 0;

        /**
         * Test if the card is DESFire by sending the getversion command.
         *
         * The optional `*uid` vector will be populate with the uid retrieve from
         * the getversion command, unless the command failed.
         */
        virtual bool is_desfire_ev1(std::vector<uint8_t> *uid = nullptr) = 0;

        /**
         * Could the card be a Mifare Classic ?
         *
         * This is less precise that the is_desfire*() function.
         */
        virtual bool maybe_mifare_classic() = 0;

      protected:
        ReaderUnit *reader_unit_;
};
}
