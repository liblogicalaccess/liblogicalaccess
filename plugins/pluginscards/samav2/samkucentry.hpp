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
			SAMKucEntry() : d_updatemask(0) { memset(&d_kucentry, 0x00, sizeof(SAMKUCEntryStruct)); };

			/**
			 * \brief Build a DESFire key given a string representation of it.
			 * \param str The string representation.
			 */
			SAMKucEntry(const SAMKUCEntryStruct& k) : d_updatemask(0), d_kucentry(k) {};

			unsigned char getUpdateMask() { return d_updatemask; };
			void setUpdateMask(unsigned char c) { d_updatemask = c; };

			KucEntryUpdateSettings getUpdateSettings();
			void setUpdateSettings(const KucEntryUpdateSettings& t);

			SAMKUCEntryStruct &getKucEntryStruct() { return d_kucentry; };
			void setKucEntryStruct(const SAMKUCEntryStruct& t) { d_kucentry = t; };

	private:

			unsigned char d_updatemask;

			SAMKUCEntryStruct d_kucentry;
	};
}

#endif /* LOGICALACCESS_DESFIREKEY_HPP */

