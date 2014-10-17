/**
 * \file mifarepluscommands.hpp
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief Mifare Plus commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCOMMANDS_HPP
#define LOGICALACCESS_MIFAREPLUSCOMMANDS_HPP

#include "mifareplusaccessinfo.hpp"
#include "logicalaccess/cards/commands.hpp"
#include "logicalaccess/cards/location.hpp"
#include "mifarepluscrypto.hpp"

#include <openssl/rand.h>
#include <openssl/aes.h>
#include "logicalaccess/crypto/aes_cipher.hpp"
#include "logicalaccess/crypto/aes_symmetric_key.hpp"
#include "logicalaccess/crypto/aes_initialization_vector.hpp"

namespace logicalaccess
{
    /**
    * \brief The Mifare Plus commands class.
    */
    class LIBLOGICALACCESS_API MifarePlusCommands : public virtual Commands
    {
    public:

        /**
        * \brief constructor
        */
        MifarePlusCommands();

        /**
        * \brief destructor
        */
        ~MifarePlusCommands();

        /**
        * \brief Generic send AES command with wrapper
        * \param command The generic command
        */
        virtual std::vector<unsigned char> AESSendCommand(const std::vector<unsigned char>& command, bool t_cl, long int /*timeout*/) = 0;

    protected:

        /**
        * \brief Generic Authentication used in all AES authentication method
        * \param keyBNr the Key Block Number
        * \param key the key
        * \param t_cl the T=CL indicator
        * \param first The first authentication indicator
        */
        virtual bool GenericAESAuthentication(unsigned short keyBNr, std::shared_ptr<MifarePlusKey> key, bool t_cl, bool first) = 0;

        /**
        * \brief Rotate the buffer by 1 from the left
        * \param string The buffer to rotate
        */
        static std::vector<unsigned char> LeftRotateSB(std::vector<unsigned char> string);

        /**
        * \brief Get a random string
        * \param init The init string for better random
        * \param size The size of the generated random string
        */
        static std::vector<unsigned char> GetRandKey(size_t size);

        /**
        * \brief Get the crypto class
        */
        std::shared_ptr<MifarePlusCrypto> GetCrypto() const;

        /**
        * \MifarePlusCrypto class instance
        */
        std::shared_ptr<MifarePlusCrypto> d_crypto;
    };
}

#endif /*LOGICALACCESS_MIFAREPLUSCOMMANDS_HPP */