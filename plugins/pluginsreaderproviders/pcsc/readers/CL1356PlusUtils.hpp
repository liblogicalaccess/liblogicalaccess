#pragma once

#include <logicalaccess/lla_fwd.hpp>

namespace logicalaccess
{
/**
 * Utilities to parse response from a
 * CL1356Plus reader.
 */
class CL1356PlusUtils
{
      public:

        /**
         * Return after a ListCard
         * command.
         */
        struct CardCount
        {
                int detected_;
                int present_;
                int active_;
        };

        static CardCount parse_list_card(const ByteVector &in);

        struct Info
        {
                /**
                 * The values match those returned by GetCardInfo (reader cmd)
                 */
                enum class State : uint8_t
                {
                        ABSENT = 0,
                        IDLE = 1,
                        READY = 2,
                        ACTIVE = 3,
                        HALT = 4,
                        PROTOCOL = 5
                };

                State state_;

                int pcd_to_picc_bit_rate_;
                int picc_to_pcd_bit_rate_;
                uint8_t card_identifier_;
                ByteVector uid_;
                ByteVector atr_;
                std::string guessed_type_;
                // more stuff we ignore
        };
        static Info parse_get_card_info(const ByteVector &in);
};
}
