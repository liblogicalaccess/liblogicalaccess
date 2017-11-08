/**
 * \file msliblogicalaccess.h
 * \author Maxime C. <maxime-dev@islog.com>
 * \brief A Visual Studio needed file.
 * The following ifdef block is the standard way of creating macros which make exporting from a DLL simpler.
 *
 * All files within this DLL are compiled with the LIBLOGICALACCESS_EXPORTS symbol defined on the command line.
 * This symbol should not be defined on any project that uses this DLL.
 * This way any other project whose source files include this file see LIBLOGICALACCESS_API functions as being imported from a DLL, whereas this DLL sees symbols defined with this macro as being exported.
 */

#ifndef MSLIBLOGICALACCESS_H
#define MSLIBLOGICALACCESS_H

#ifdef LIBLOGICALACCESS_EXPORTS
#define LIBLOGICALACCESS_API __declspec(dllexport)
#else
/**
 * \brief A MACRO usefull to use the same include files to compile and to use the library.
 */
#define LIBLOGICALACCESS_API __declspec(dllimport)
#endif

#include <logicalaccess/msvc_boost_disable.h>

#endif /* MSLIBLOGICALACCESS_H */
