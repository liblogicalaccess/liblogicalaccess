/**
 * \file MifarePlusCommandsSL1.h
 * \author Xavier S. <xavier.schmerber@gmail.com>
 * \brief MifarePlus SL1 commands.
 */

#ifndef LOGICALACCESS_MIFAREPLUSCOMMANDSSL1_H
#define LOGICALACCESS_MIFAREPLUSCOMMANDSSL1_H

#include "../Mifare/MifareCommands.h"
#include "MifarePlusCommands.h"
#include "MifarePlusAccessInfo.h"


namespace LOGICALACCESS
{
	/**
	 * \brief The MifarePlus SL1 commands class.
	 */
	class LIBLOGICALACCESS_API MifarePlusCommandsSL1 : public virtual MifareCommands, public virtual MifarePlusCommands
	{
		public:

			/**
			 * \brief Load a key on a given location.
			 * \param location The location.
			 * \param key The key.
			 * \param keytype The mifare plus key type.
			 */
			virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifarePlusKeyType keytype) = 0;

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param key_storage The key storage used for authentication.
			 * \param keytype The mifare plus key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifarePlusKeyType keytype) = 0;

			/**
			* \brief Switch to SL2
			* \param key The switch SL2 AES key
			*/
			virtual bool SwitchLevel2(boost::shared_ptr<MifarePlusKey> key) = 0;

			virtual bool AESAuthenticate(boost::shared_ptr<MifarePlusKey> key) = 0;
			

		protected:
			
	};
}

#endif /* LOGICALACCESS_MIFAREPLUSCOMMANDSSL1_H */
