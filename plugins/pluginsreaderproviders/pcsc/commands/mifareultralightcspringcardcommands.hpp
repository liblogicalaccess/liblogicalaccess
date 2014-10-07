/**
 * \file mifareultralightcacscommands.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight C - SpringCard.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP
#define LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP

#include "mifareultralightcpcsccommands.hpp"


namespace logicalaccess
{
	/**
	 * \brief The Mifare Ultralight C commands class for SpringCard reader.
	 */
	class LIBLOGICALACCESS_API MifareUltralightCSpringCardCommands : public MifareUltralightCPCSCCommands
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightCSpringCardCommands();

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightCSpringCardCommands();

		
		protected:

            virtual std::vector<unsigned char> sendGenericCommand(const std::vector<unsigned char>& data);
					
	};	
}

#endif /* LOGICALACCESS_MIFAREULTRALIGHTCSPRINGCARDCOMMANDS_HPP */

