/**
 * \file MifareSpringCardCommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare SpringCard card.
 */

#ifndef LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL1_HPP
#define LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL1_HPP

#include "mifareplusspringcardcommands.hpp"
#include "../readercardadapters/pcscreadercardadapter.hpp"
#include "mifareplussl1commands.hpp"
#include "mifarespringcardcommands.hpp"


#include <string>
#include <vector>
#include <iostream>


namespace logicalaccess
{
	/**
	 * \brief The Mifare Plus commands SL1 class for SpringCard reader.
	 */
	class LIBLOGICALACCESS_API MifarePlusSpringCardCommandsSL1 : public MifarePlusSL1Commands, public MifarePlusSpringCardCommands, public MifareSpringCardCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifarePlusSpringCardCommandsSL1();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifarePlusSpringCardCommandsSL1();

		
		protected:						
			
			/**
			 * \brief Load a key on a given location.
			 * \param location The location.
			 * \param key The key.
			 * \param keytype The mifare plus key type.
			 */
			virtual void loadKey(boost::shared_ptr<Location> location, boost::shared_ptr<Key> key, MifarePlusKeyType keytype);

			/**
			 * \brief Authenticate a block, given a key number.
			 * \param blockno The block number.
			 * \param key_storage The key storage used for authentication.
			 * \param keytype The mifare plus key type.
			 * \return true if authenticated, false otherwise.
			 */
			virtual void authenticate(unsigned char blockno, boost::shared_ptr<KeyStorage> key_storage, MifarePlusKeyType keytype);

			/**
			* \brief Convert MifarePlus key to Mifare key
			*/
			boost::shared_ptr<MifareKey> ConvertKey(boost::shared_ptr<Key> key);

			/**
			* \brief Switch to SL2
			* \param key The switch SL2 AES key
			*/
			virtual bool SwitchLevel2(boost::shared_ptr<MifarePlusKey> key);

			/**
			* \brief Switch to SL3
			* \param key The switch SL3 AES key
			*/
			virtual bool SwitchLevel3(boost::shared_ptr<MifarePlusKey> key);

			/**
			* \brief authenticate to SL1 with AES security
			* \param key The SL1 specific AES key
			*/
			virtual bool AESAuthenticate(boost::shared_ptr<MifarePlusKey> key);
	};	
}

#endif /* LOGICALACCESS_MIFAREPLUSSPRINGCARDCOMMANDSSL1_HPP */

