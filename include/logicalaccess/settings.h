/**
 * \file settings.h
 * \author Maxime CHAMLEY <maxime.chamley@islog.eu>
 * \brief Settings header
 */

#ifndef LOGICALACCESS_SETTINGS_H
#define LOGICALACCESS_SETTINGS_H

#include <string>
#include <sstream>
#include <vector>
#ifndef LIBLOGICALACCESS_API
#ifdef _MSC_VER
#include "msliblogicalaccess.h"
#else
#define LIBLOGICALACCESS_API
#endif
#endif

namespace LOGICALACCESS
{
	class LIBLOGICALACCESS_API Settings
	{
		public:

			static Settings& getInstance();
			
			void Initialize();  
			void Uninitialize();  

			/* Logs */
			bool IsLogEnabled;
			std::string LogFileName;
			bool SeeWaitInsertionLog;
			bool SeeWaitRemovalLog;
			bool SeeCommunicationLog;

			/* Auto-Detection */
			bool IsAutoDetectEnabled;
			long int AutoDetectionTimeout;

			/* Serial port configuration */
			bool IsConfigurationRetryEnabled;
			long int ConfigurationRetryTimeout;

			/* Default Reader */
			std::string DefaultReader;
			std::vector<std::string> PluginFolders;

		protected:

			Settings();
			
			void LoadSettings();
			void SaveSettings();

			static Settings instance;
			
		private:
		
			void reset();
			std::string getDllPath();
	};
}

#endif