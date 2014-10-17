/**
 * \file mifarespringcardcommandssl3.hpp
 * \author Xavier SCHMERBER <xavier.schmerber@gmail.com>
 * \brief Mifare Plus SpringCard card.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL3_HPP
#define LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL3_HPP

#include "mifareplusspringcardcommands.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "mifareplussl3commands.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
    /**
     * \brief The Mifare Plus commands SL3 class for SpringCard reader.
     */
    class LIBLOGICALACCESS_API MifarePlusSpringCardCommandsSL3 : public MifarePlusSL3Commands, public MifarePlusSpringCardCommands
    {
    public:

        /**
         * \brief Constructor.
         */
        MifarePlusSpringCardCommandsSL3();

        /**
         * \brief Destructor.
         */
        virtual ~MifarePlusSpringCardCommandsSL3();

        /**
         * \brief Authenticate a block, given a key number.
         * \param sector The sector to authenticate to (null if key is not a sector key)
         * \param key The key to use for authentication.
         * \param keytype The mifare plus key type.
         * \return true if authenticated, false otherwise.
         */
        virtual bool followingAuthenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype);

        /**
         * \brief Authenticate a block, given a key number.
         * \param sector The sector to authenticate to (null if key is not a sector key)
         * \param key The key to use for authentication.
         * \param keytype The mifare plus key type.
         * \return true if authenticated, false otherwise.
         */
        virtual bool authenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype);

        /**
        * \brief Reset the authentication
        */
        virtual void resetAuthentication();

        /**
         * \brief Read bytes from the card.
         * \param blockno The block number.
         * \param len The number of blocks to read. (0 <= len < 16)
         * \param encrypted The encryption indicator
         * \param macOnCommand The mac on command indicator
         * \param macOnResponse The mac on response indicator
         * \param buf The buffer in which to place the data.
         * \param buflen The length of buffer.
         * \return The count of bytes red.
         */
        virtual size_t readBinary(short blockno, char len, bool encrypted, bool macOnCommand, bool macOnResponse, void* buf, size_t buflen);

        /**
         * \brief Read bytes from the card.
         * \param blockno The block number.
         * \param encrypted The encryption indicator
         * \param macOnResponse The mac on response indicator
         * \param buf The buffer containing the data to write
         * \param buflen The length of buffer.
         * \return The count of bytes red.
         */
        virtual size_t updateBinary(short blockno, bool encrypted, bool macOnResponse, const void* buf, size_t buflen);

        /**
         * \brief Increment a block
         * \param blockno The block number of the block to be incremented.
         * \param macOnResponse The mac on response indicator
         * \param value The data to add
         * \return true if operation was successfull
         */
        virtual bool increment(short blockno, bool macOnResponse, int value);

        /**
         * \brief Decrement a block
         * \param blockno The block number of the block to be decremented.
         * \param macOnResponse The mac on response indicator
         * \param value The data to add
         * \return true if operation was successfull
         */
        virtual bool decrement(short blockno, bool macOnResponse, int value);

        /**
         * \brief Transfer a block
         * \param blockno The block number of the destination block
         * \param macOnResponse The mac on response indicator
         * \return true if operation was successfull
         */
        virtual bool transfer(short blockno, bool macOnResponse);

        /**
         * \brief Increment aand transfer a block
         * \param sourceBlockno The block number of the source block to be incremented.
         * \param destBlockno The block number of the destination block to be incremented.
         * \param macOnResponse The mac on response indicator
         * \param value The data to add
         * \return true if operation was successfull
         */
        virtual bool incrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value);

        /**
         * \brief decrement aand transfer a block
         * \param sourceBlockno The block number of the source block to be decremented.
         * \param destBlockno The block number of the destination block to be decremented.
         * \param macOnResponse The mac on response indicator
         * \param value The data to add
         * \return true if operation was successfull
         */
        virtual bool decrementTransfer(short sourceBlockno, short destBlockno, bool macOnResponse, int value);

        /**
         * \brief Restore a block
         * \param blockno The block number of the to be restored block
         * \param macOnResponse The mac on response indicator
         * \return true if operation was successfull
         */
        virtual bool restore(short blockno, bool macOnResponse);

        /**
        * \brief Check the card proximity
        * \param auth The indicator of active session (protocol differs if card is authenticated)
        */
        //virtual bool proximityCheck(bool auth);
        //TODO : find the PPS1 and calculate the mac to use the proximity check

    protected:

        /**
         * \brief Authenticate a block, given a key number.
         * \param sector The sector to authenticate to (null if key is not a sector key)
         * \param key The key to use for authentication.
         * \param keytype The mifare plus key type.
         * \param first The first authentication indicator
         * \return true if authenticated, false otherwise.
         */
        virtual bool genericAuthenticate(int sector, std::shared_ptr<MifarePlusKey> key, MifarePlusKeyType keytype, bool first);

        /**
         * \brief Get the appropriate read command code
         * \param encrypted The encryption indicator
         * \param macOnCommand The mac on command indicator
         * \param macOnResponse The mac on response indicator
         */
        unsigned char findReadCommandCode(bool encrypted, bool macOnCommand, bool macOnResponse);

        /**
         * \brief Get the appropriate write command code
         * \param encrypted The encryption indicator
         * \param macOnResponse The mac on response indicator
         */
        unsigned char findWriteCommandCode(bool encrypted, bool macOnResponse);

        /**
         * \brief Get the MAC
         */
        std::vector<unsigned char> getMAC(std::vector<unsigned char> command);

        /**
         * \brief Check the MAC
         * \param res The card MACed response
         * \param command The command sent to the card
         */
        std::vector<unsigned char> checkMAC(std::vector<unsigned char> res, std::vector<unsigned char> command);
    };
}

#endif /* LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL3_HPP */