/**
 * \file desfirekey.hpp
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief DESFire key.
 */

#ifndef LOGICALACCESS_SAMKUCENTRY_HPP
#define LOGICALACCESS_SAMKUCENTRY_HPP

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


	typedef struct s_SAMKUCEntry
	{
		unsigned char	limit[4];
		unsigned char	keynockuc;
		unsigned char	keyvckuc;
		unsigned char	curval[4];
	}				SAMKUCEntryStruct;

	/**
	 * \brief A DESFire Key class.
	 */
	class LIBLOGICALACCESS_API SAMKucEntry
	{
		public:

			/**
			 * \brief Build an empty DESFire key.
			 */
			SAMKucEntry() : d_updatemask(0), d_kucentry(new SAMKUCEntryStruct) { memset(&*d_kucentry, 0, sizeof(SAMKUCEntryStruct)); };

			/**
			 * \brief Build a DESFire key given a string representation of it.
			 * \param str The string representation.
			 */
			SAMKucEntry(boost::shared_ptr<SAMKUCEntryStruct> k) : d_updatemask(0), d_kucentry(k) {};

			unsigned char	getUpdateMask() { return d_updatemask; };
			void			setUpdateMask(unsigned char c) { d_updatemask = c; };

			boost::shared_ptr<KucEntryUpdateSettings> getUpdateSettings();
			void	setUpdateSettings(boost::shared_ptr<KucEntryUpdateSettings> t);

			boost::shared_ptr<SAMKUCEntryStruct> getKucEntryStruct() { return d_kucentry; };
			void									setKucEntryStruct(boost::shared_ptr<SAMKUCEntryStruct> t) { d_kucentry = t; };

	private:

			unsigned char d_updatemask;

			boost::shared_ptr<SAMKUCEntryStruct> d_kucentry;
	};
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */

