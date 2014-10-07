/**
 * \file mifareultralightcpcsccommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C pcsc card.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP

#include "mifareultralightpcsccommands.hpp"
#include "mifareultralightccommands.hpp"


namespace logicalaccess
{
	/**
	 * \brief The Mifare Ultralight C commands class for PCSC reader.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCPCSCCommands : public MifareUltralightPCSCCommands, public MifareUltralightCCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightCPCSCCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightCPCSCCommands();

			/**
			 * \brief Authenticate to the chip.
			 * \param authkey The authentication key.
			 */
			virtual void authenticate(boost::shared_ptr<TripleDESKey> authkey);

		
		protected:

            virtual std::vector<unsigned char> sendGenericCommand(const std::vector<unsigned char>& data);

            virtual std::vector<unsigned char> authenticate_PICC1();

            virtual std::vector<unsigned char> authenticate_PICC2(const std::vector<unsigned char>& encRndAB);
					
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCPCSCCOMMANDS_HPP */

