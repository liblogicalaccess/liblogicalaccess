/**
 * \file desfireev1cardprovider.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire EV1 card provider.
 */

#ifndef LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP
#define LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP

#include "desfirecardprovider.hpp"
#include "desfireev1commands.hpp"


namespace logicalaccess
{
	class DESFireEV1Chip;

	/**
	 * \brief The DESFire base card provider class.
	 */
	class LIBLOGICALACCESS_API DESFireEV1CardProvider : public DESFireCardProvider
	{
		public:

			/**
			 * \brief Select an application.
			 * \param location The DESFire location
			 */
			virtual void selectApplication(boost::shared_ptr<DESFireLocation> location);

			/**
			 * \brief Create a new application.
			 * \param location The DESFire location
			 * \param settings Key settings
			 * \param maxNbKeys Maximum number of keys
			 */
			virtual void createApplication(boost::shared_ptr<DESFireLocation> location, DESFireKeySettings settings, int maxNbKeys);

			/**
			 * \brief Create a new data file in the current application.
			 * \param location The DESFire location
			 * \param accessRights The file access rights
			 * \param fileSize The file size.
			 */
			virtual void createStdDataFile(boost::shared_ptr<DESFireLocation> location, DESFireAccessRights accessRights, int fileSize);

			boost::shared_ptr<DESFireEV1Chip> getDESFireEV1Chip();

			boost::shared_ptr<DESFireEV1Commands> getDESFireEV1Commands() const { return boost::dynamic_pointer_cast<DESFireEV1Commands>(getCommands()); };

		protected:

				
	};
}

#endif /* LOGICALACCESS_DESFIREEV1CARDPROVIDER_HPP */

