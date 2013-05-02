cmake_minimum_required(VERSION 2.6)

project(liblogicalaccess)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

if(UNIX)
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/packages/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/packages/lib")
else()
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/packages/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH}")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(VC_FOLDER_NAME "Debug")
else()
	set(VC_FOLDER_NAME "Release")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG -D_DEBUG)
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  
  if (TARGET_ARCH STREQUAL "Win32")
    add_definitions(-DWIN32 -D_WIN32)
  else()
	add_definitions(-DWIN64 -D_WIN64)
  endif()
  add_definitions(-DLOGICALACCESS_LOGS -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS -DLIBLOGICALACCESS_EXPORTS -DLIBLOGICALACCESS_EXPORTS_DLL)
endif()

if(UNIX)
	SET(LIBRARY_OUTPUT_PATH ./bin)
else()
	SET(LIBRARY_OUTPUT_PATH ./bin/${TARGET_ARCH})
endif()

SET(NATIVE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(NATIVE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Cryptolib/${VC_FOLDER_NAME})

#Source Files
file(GLOB source_root ${NATIVE_SOURCE}/*.cpp)
file(GLOB source_cards ${NATIVE_SOURCE}/Cards/*.cpp)
file(GLOB source_readerproviders ${NATIVE_SOURCE}/ReaderProviders/*.cpp)
file(GLOB source_service ${NATIVE_SOURCE}/Services/*.cpp)
file(GLOB source_accesscontrol ${NATIVE_SOURCE}/Services/AccessControl/*.cpp)
file(GLOB source_encodings ${NATIVE_SOURCE}/Services/AccessControl/Encodings/*.cpp)
file(GLOB source_formats ${NATIVE_SOURCE}/Services/AccessControl/Formats/*.cpp)
file(GLOB source_customformat ${NATIVE_SOURCE}/Services/AccessControl/Formats/CustomFormat/*.cpp)
file(GLOB source_nfctag ${NATIVE_SOURCE}/Services/NFCTag/*.cpp)

			
#Include Files
file(GLOB include_root ${NATIVE_INCLUDE}/logicalaccess/*.h)
file(GLOB include_cards ${NATIVE_INCLUDE}/logicalaccess/Cards/*.h)
file(GLOB include_readerproviders ${NATIVE_INCLUDE}/logicalaccess/ReaderProviders/*.h)
file(GLOB include_readerproviders_pcsc ${NATIVE_INCLUDE}/logicalaccess/ReaderProviders/PCSC/*.h)
file(GLOB include_service ${NATIVE_INCLUDE}/logicalaccess/Services/*.h)
file(GLOB include_accesscontrol ${NATIVE_INCLUDE}/logicalaccess/Services/AccessControl/*.h)
file(GLOB include_encodings ${NATIVE_INCLUDE}/logicalaccess/Services/AccessControl/Encodings/*.h)
file(GLOB include_formats ${NATIVE_INCLUDE}/logicalaccess/Services/AccessControl/Formats/*.h)
file(GLOB include_customformat ${NATIVE_INCLUDE}/logicalaccess/Services/AccessControl/Formats/CustomFormat/*.h)
file(GLOB include_nfctag ${NATIVE_INCLUDE}/logicalaccess/Services/NFCTag/*.h)

#Includes Plugins Files
file(GLOB_RECURSE include_plugins ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/*.h)

INCLUDE_DIRECTORIES(${NATIVE_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Cryptolib/include)

if(UNIX)

file(GLOB source_dynlibrary ${NATIVE_SOURCE}/DynLibrary/PosixDynLibrary.cpp ${NATIVE_SOURCE}/DynLibrary/LibraryManager.cpp)
INCLUDE_DIRECTORIES("/usr/local/include")
file(GLOB include_dynlibrary ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/PosixDynLibrary.h ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/IDynLibrary.h ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/LibraryManager.h)

else()

file(GLOB source_vc ${CMAKE_CURRENT_SOURCE_DIR}/vc++/*.cpp)
file(GLOB source_dynlibrary ${NATIVE_SOURCE}/DynLibrary/WindowsDynLibrary.cpp ${NATIVE_SOURCE}/DynLibrary/WinClass.cpp ${NATIVE_SOURCE}/DynLibrary/LibraryManager.cpp)

file(GLOB include_dynlibrary ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/WinClass.h ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/IDynLibrary.h ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/WindowsDynLibrary.h ${NATIVE_INCLUDE}/logicalaccess/DynLibrary/LibraryManager.h)

SOURCE_GROUP("Source Files\\Cards" FILES ${source_cards})
SOURCE_GROUP("Source Files\\DynLibrary" FILES ${source_dynlibrary})
SOURCE_GROUP("Source Files\\ReaderProviders" FILES ${source_readerproviders})
SOURCE_GROUP("Source Files\\Services" FILES ${source_service})
SOURCE_GROUP("Source Files\\Services\\AccessControl" FILES ${source_accesscontrol})
SOURCE_GROUP("Source Files\\Services\\AccessControl\\Encodings" FILES ${source_encodings})
SOURCE_GROUP("Source Files\\Services\\AccessControl\\Formats" FILES ${source_formats})
SOURCE_GROUP("Source Files\\Services\\AccessControl\\Formats\\CustomFormat" FILES ${source_customformat})
SOURCE_GROUP("Source Files\\Services\\NFCTag" FILES ${source_nfctag})
			
SOURCE_GROUP("Header Plugins" FILES ${include_plugins})
SOURCE_GROUP("Header Files\\Cards" FILES ${include_cards})
SOURCE_GROUP("Header Files\\DynLibrary" FILES ${include_dynlibrary})
SOURCE_GROUP("Header Files\\ReaderProviders" FILES ${include_readerproviders})
SOURCE_GROUP("Header Files\\ReaderProviders\\PCSC" FILES ${include_readerproviders_pcsc})
SOURCE_GROUP("Header Files\\Services" FILES ${include_service})
SOURCE_GROUP("Header Files\\Services\\AccessControl" FILES ${include_accesscontrol})
SOURCE_GROUP("Header Files\\Services\\AccessControl\\Encodings" FILES ${include_encodings})
SOURCE_GROUP("Header Files\\Services\\AccessControl\\Formats" FILES ${include_formats})
SOURCE_GROUP("Header Files\\Services\\AccessControl\\Formats\\CustomFormat" FILES ${include_customformat})
SOURCE_GROUP("Header Files\\Services\\NFCTag" FILES ${include_nfctag})

endif()

set(source ${source_vc} ${source_root} ${source_cards} ${source_crypto} ${source_readerproviders}  ${source_service} ${source_accesscontrol} ${source_encodings} ${source_formats} ${source_customformat} ${source_nfctag} ${source_dynlibrary})

set(include ${include_vc} ${include_root} ${include_cards} ${include_crypto} ${include_readerproviders} ${include_service} ${include_accesscontrol} ${include_encodings} ${include_formats} ${include_customformat} ${include_nfctag} ${include_plugins} ${include_dynlibrary})

LINK_DIRECTORIES( ${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Cryptolib/${VC_FOLDER_NAME}")

add_library(
  liblogicalaccess
  SHARED
  ${source}
  ${include}
  LICENSE.LibLogicalAccess.txt
)


if (WIN32)

set(SCLIB_ENDNAME)
if (TARGET_ARCH STREQUAL "x64")
	set (SCLIB_ENDNAME "x64")
endif()

target_link_libraries(liblogicalaccess winscard zlib libeay32 ssleay32 scardsyn${SCLIB_ENDNAME} Winmm ws2_32 delayimp logicalaccess-cryptolib)

endif()

add_subdirectory(Plugins/Cryptolib)
add_subdirectory(samples/basic)