/**
 * \file mifareplusspringcardcommands.hpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare Plus SpringCard card.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDS_HPP

#include "mifarepluscommands.hpp"
#include "../../pluginsreaderproviders/pcsc/readercardadapters/pcscreadercardadapter.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare Plus commands class for SpringCard reader.
     */
    class LIBLOGICALACCESS_API MifarePlusSpringCardCommands : public virtual MifarePlusCommands
    {
    public:

        /**
        * \brief constructor
        */
        MifarePlusSpringCardCommands();

        /**
        * \brief destructor
        */
        ~MifarePlusSpringCardCommands();

        /**
        * \brief Generic send AES command with wrapper
        * \param command The generic command
        */
        virtual std::vector<unsigned char> AESSendCommand(const std::vector<unsigned char>& command, bool t_cl, long int /*timeout*/);

    protected:

        /**
        * \brief Generic Authentication used in all AES authentication method
        * \param keyBNr the Key Block Number
        * \param key the key
        * \param t_cl the T=CL indicator
        * \param first The first authentication indicator
        */
        virtual bool GenericAESAuthentication(unsigned short keyBNr, boost::shared_ptr<MifarePlusKey> key, bool t_cl, bool first);

        /**
        * \brief Turn on the TCL mode
        */
        virtual bool TurnOnTCL();

        /**
        * \brief Turn off the TCL mode
        */
        virtual bool TurnOffTCL();

        /**
        * \brief Wake up the card (TCL mode must be turned off)
        */
        virtual bool WakeUpCard();

        /**
        * \brief Check if the card returned an error code
        * \param result The string returned by the card
        * \param resultlen The length of the result
        */
        static void CheckCardReturn(const unsigned char result[], size_t resultlen);
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDS_H */