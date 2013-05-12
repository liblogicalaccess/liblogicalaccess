/**
 * \file mifareultralightchip.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief Mifare Ultralight chip.
 */

#ifndef LOGICALACCESS_MIFAREULTRALIGHTCHIP_H
#define LOGICALACCESS_MIFAREULTRALIGHTCHIP_H

#include "logicalaccess/cards/chip.hpp"

#include "mifareultralightcardprovider.hpp"
#include "mifareultralightprofile.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace logicalaccess
{
	#define CHIP_MIFAREULTRALIGHT		"MifareUltralight"

	/**
	 * \brief The Mifare Ultralight base chip class.
	 */
	class LIBLOGICALACCESS_API MifareUltralightChip : public Chip
	{
		public:			

			/**
			 * \brief Constructor.
			 */
			MifareUltralightChip();

			/**
			 * \brief Constructor.
			 * \param ct The card type.
			 */
			MifareUltralightChip(std::string ct);

			/**
			 * \brief Destructor.
			 */
			virtual ~MifareUltralightChip();

			/**
			 * \brief Get the generic card type.
			 * \return The generic card type.
			 */
			virtual std::string getGenericCardType() const { return CHIP_MIFAREULTRALIGHT; };			

			/**
			 * \brief Get the root location node.
			 * \return The root location node.
			 */
			virtual boost::shared_ptr<LocationNode> getRootLocationNode();

			/**
			 * \brief Get the Mifare Ultralight card provider for I/O access.
			 * \return The Mifare Ultralight card provider.
			 */
			boost::shared_ptr<MifareUltralightCardProvider> getMifareUltralightCardProvider() { return boost::dynamic_pointer_cast<MifareUltralightCardProvider>(getCardProvider()); };

			/**
			 * \brief Get the Mifare Ultralight profile.
			 * \return The Mifare Ultralight profile.
			 */
			boost::shared_ptr<MifareUltralightProfile> getMifareUltralightProfile() { return boost::dynamic_pointer_cast<MifareUltralightProfile>(getProfile()); };

		protected:

			void addPageNode(boost::shared_ptr<LocationNode> rootNode, int page);
	};
}

#endif
