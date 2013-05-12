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

SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/${VC_FOLDER_NAME})

#Source Files
file(GLOB source_root ${NATIVE_SOURCE}/*.cpp)
file(GLOB source_cards ${NATIVE_SOURCE}/cards/*.cpp)
file(GLOB source_readerproviders ${NATIVE_SOURCE}/readerproviders/*.cpp)
file(GLOB source_service ${NATIVE_SOURCE}/services/*.cpp)
file(GLOB source_accesscontrol ${NATIVE_SOURCE}/services/accesscontrol/*.cpp)
file(GLOB source_encodings ${NATIVE_SOURCE}/services/accesscontrol/encodings/*.cpp)
file(GLOB source_formats ${NATIVE_SOURCE}/services/accesscontrol/formats/*.cpp)
file(GLOB source_customformat ${NATIVE_SOURCE}/services/accesscontrol/formats/customformat/*.cpp)
file(GLOB source_nfctag ${NATIVE_SOURCE}/services/nfctag/*.cpp)

			
#Include Files
file(GLOB include_root ${NATIVE_INCLUDE}/logicalaccess/*.h)
file(GLOB include_root ${NATIVE_INCLUDE}/logicalaccess/*.hpp)
file(GLOB include_cards ${NATIVE_INCLUDE}/logicalaccess/cards/*.hpp)
file(GLOB include_readerproviders ${NATIVE_INCLUDE}/logicalaccess/readerproviders/*.hpp)
file(GLOB include_readerproviders_pcsc ${NATIVE_INCLUDE}/logicalaccess/readerProviders/pcsc/*.hpp)
file(GLOB include_service ${NATIVE_INCLUDE}/logicalaccess/services/*.hpp)
file(GLOB include_accesscontrol ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/*.hpp)
file(GLOB include_encodings ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/encodings/*.hpp)
file(GLOB include_formats ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/formats/*.hpp)
file(GLOB include_customformat ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/formats/customformat/*.hpp)
file(GLOB include_nfctag ${NATIVE_INCLUDE}/logicalaccess/services/nfctag/*.hpp)

#Includes Plugins Files
file(GLOB_RECURSE include_plugins ${CMAKE_CURRENT_SOURCE_DIR}/plugins/*.hpp)

INCLUDE_DIRECTORIES(${NATIVE_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/include)

if(UNIX)

file(GLOB source_dynlibrary ${NATIVE_SOURCE}/dynlibrary/posixdynlibrary.cpp ${NATIVE_SOURCE}/dynlibrary/librarymanager.cpp)
INCLUDE_DIRECTORIES("/usr/local/include")
file(GLOB include_dynlibrary ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/posixdynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/idynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/librarymanager.hpp)

else()

file(GLOB source_vc ${CMAKE_CURRENT_SOURCE_DIR}/vc++/*.cpp)
file(GLOB source_dynlibrary ${NATIVE_SOURCE}/dynlibrary/windowsdynlibrary.cpp ${NATIVE_SOURCE}/dynlibrary/winclass.cpp ${NATIVE_SOURCE}/dynlibrary/librarymanager.cpp)

file(GLOB include_dynlibrary ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/winclass.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/idynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/windowsdynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/librarymanager.hpp)

SOURCE_GROUP("Source Files\\cards" FILES ${source_cards})
SOURCE_GROUP("Source Files\\dynlibrary" FILES ${source_dynlibrary})
SOURCE_GROUP("Source Files\\readerproviders" FILES ${source_readerproviders})
SOURCE_GROUP("Source Files\\services" FILES ${source_service})
SOURCE_GROUP("Source Files\\services\\accesscontrol" FILES ${source_accesscontrol})
SOURCE_GROUP("Source Files\\services\\accesscontrol\\encodings" FILES ${source_encodings})
SOURCE_GROUP("Source Files\\services\\accesscontrol\\formats" FILES ${source_formats})
SOURCE_GROUP("Source Files\\services\\accesscontrol\\formats\\customformat" FILES ${source_customformat})
SOURCE_GROUP("Source Files\\services\\nfctag" FILES ${source_nfctag})
			
SOURCE_GROUP("Header Plugins" FILES ${include_plugins})
SOURCE_GROUP("Header Files\\cards" FILES ${include_cards})
SOURCE_GROUP("Header Files\\dynlibrary" FILES ${include_dynlibrary})
SOURCE_GROUP("Header Files\\readerproviders" FILES ${include_readerproviders})
SOURCE_GROUP("Header Files\\readerproviders\\pcsc" FILES ${include_readerproviders_pcsc})
SOURCE_GROUP("Header Files\\services" FILES ${include_service})
SOURCE_GROUP("Header Files\\services\\accesscontrol" FILES ${include_accesscontrol})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\encodings" FILES ${include_encodings})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats" FILES ${include_formats})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats\\customformat" FILES ${include_customformat})
SOURCE_GROUP("Header Files\\services\\nfctag" FILES ${include_nfctag})

endif()

set(source ${source_vc} ${source_root} ${source_cards} ${source_crypto} ${source_readerproviders}  ${source_service} ${source_accesscontrol} ${source_encodings} ${source_formats} ${source_customformat} ${source_nfctag} ${source_dynlibrary})

set(include ${include_vc} ${include_root} ${include_cards} ${include_crypto} ${include_readerproviders} ${include_service} ${include_accesscontrol} ${include_encodings} ${include_formats} ${include_customformat} ${include_nfctag} ${include_plugins} ${include_dynlibrary})

LINK_DIRECTORIES( ${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/${VC_FOLDER_NAME}")

add_library(
  liblogicalaccess
  SHARED
  ${source}
  ${include}
  LICENSE.LibLogicalAccess.txt
)

SET_TARGET_PROPERTIES(liblogicalaccess PROPERTIES OUTPUT_NAME "liblogicalaccess" PREFIX "" VERSION 2.0.3)

if (WIN32)

  set(SCLIB_ENDNAME)
  if (TARGET_ARCH STREQUAL "x64")
	set (SCLIB_ENDNAME "x64")
  endif()

  target_link_libraries(liblogicalaccess winscard zlib libeay32 ssleay32 scardsyn${SCLIB_ENDNAME} Winmm ws2_32 delayimp logicalaccess-cryptolib)

elseif (UNIX)

  target_link_libraries(liblogicalaccess logicalaccess-cryptolib crypto boost_filesystem boost_date_time boost_system dl)
endif()

add_subdirectory(plugins/cryptolib)
add_subdirectory(samples/basic)