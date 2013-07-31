/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_SAMAV2KUCENTRY_HPP
#define LOGICALACCESS_SAMAV2KUCENTRY_HPP

#include "logicalaccess/key.hpp"

namespace logicalaccess
{
	typedef struct  s_KucEntryUpdateSettings
	{
		char		limit;
		char		keynockuc;
		char		keyvckuc;
		char		rfu[5];
	}				KucEntryUpdateSettings;


	typedef struct s_SAMAV2KUCEntry
	{
		unsigned char	limit[4];
		unsigned char	keynockuc;
		unsigned char	keyvckuc;
		unsigned char	curval[4];
	}				SAMAV2KUCEntryStruct;

	/**
	 * \brief A DESFire Key class.
	 */
	class LIBLOGICALACCESS_API SAMAV2KucEntry
	{
		public:

			/**
			 * \brief Build an empty DESFire key.
			 */
			SAMAV2KucEntry() : d_kucentry(new SAMAV2KUCEntryStruct) {};

			/**
			 * \brief Build a DESFire key given a string representation of it.
			 * \param str The string representation.
			 */
			SAMAV2KucEntry(boost::shared_ptr<SAMAV2KUCEntryStruct> k) : d_kucentry(k) {};

			unsigned char	getUpdateMask() { return d_updatemask; };
			void			setUpdateMask(unsigned char c) { d_updatemask = c; };

			boost::shared_ptr<KucEntryUpdateSettings> getUpdateSettings();
			void	setUpdateSettings(boost::shared_ptr<KucEntryUpdateSettings> t);

			boost::shared_ptr<SAMAV2KUCEntryStruct> getKucEntryStruct() { return d_kucentry; };
			void									setKucEntryStruct(boost::shared_ptr<SAMAV2KUCEntryStruct> t) { d_kucentry = t; };

	private:

			unsigned char d_updatemask;

			boost::shared_ptr<SAMAV2KUCEntryStruct> d_kucentry;
	};
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */

