cmake_minimum_required(VERSION 2.6)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" )
endif()


project(liblogicalaccess)

set(LIBLOGICALACCESS_VERSION_MAJOR 1)
set(LIBLOGICALACCESS_VERSION_MINOR 74)
set(LIBLOGICALACCESS_VERSION_STRING ${LIBLOGICALACCESS_VERSION_MAJOR}.${LIBLOGICALACCESS_VERSION_MINOR} )

list ( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

option ( BUILD_SHARED_LIBS "build shared libraries" ON )

find_package(CXX11 REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

find_package(Boost 1.49 REQUIRED COMPONENTS filesystem system date_time)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

if (UNIX)
  find_package(PCSCLite REQUIRED)
  include_directories(${PCSCLITE_INCLUDE_DIRS})
endif()

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

if (MSVC)
    SET(BOOST_LIBRARYDIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH})
    SET(BOOST_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/include)
    SET(Boost_USE_STATIC_LIBS ON)

    SET(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH})
    SET(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/include)

    SET(ZLIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH}/zlib.lib)
    SET(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/include)
endif()

include_directories(packages/include)
if(UNIX)
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/packages/lib")
else()
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
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
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
file(GLOB include_root ${NATIVE_INCLUDE}/logicalaccess/*.h*)
file(GLOB include_cards ${NATIVE_INCLUDE}/logicalaccess/cards/*.hpp)
file(GLOB include_readerproviders ${NATIVE_INCLUDE}/logicalaccess/readerproviders/*.hpp)
file(GLOB include_service ${NATIVE_INCLUDE}/logicalaccess/services/*.hpp)
file(GLOB include_accesscontrol ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/*.hpp)
file(GLOB include_encodings ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/encodings/*.hpp)
file(GLOB include_formats ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/formats/*.hpp)
file(GLOB include_customformat ${NATIVE_INCLUDE}/logicalaccess/services/accesscontrol/formats/customformat/*.hpp)
file(GLOB include_nfctag ${NATIVE_INCLUDE}/logicalaccess/services/nfctag/*.hpp)
file(GLOB include_storage ${NATIVE_INCLUDE}/logicalaccess/services/storage/*.hpp)

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
SOURCE_GROUP("Header Files\\services" FILES ${include_service})
SOURCE_GROUP("Header Files\\services\\accesscontrol" FILES ${include_accesscontrol})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\encodings" FILES ${include_encodings})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats" FILES ${include_formats})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats\\customformat" FILES ${include_customformat})
SOURCE_GROUP("Header Files\\services\\nfctag" FILES ${include_nfctag})

endif()

install ( FILES LICENSE.LibLogicalAccess.txt DESTINATION share/licenses/liblogicalaccess )

set(LIBLOGICALACCESS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/logicalaccess)

install ( FILES ${include_root} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR} )
install ( FILES ${include_cards} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/cards )
install ( FILES ${include_readerproviders} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/readerproviders )
install ( FILES ${include_service} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/services )
install ( FILES ${include_accesscontrol} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/accesscontrol )
install ( FILES ${include_encodings} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/accesscontrol/encodings )
install ( FILES ${include_formats} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/accesscontrol/formats )
install ( FILES ${include_customformat} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/accesscontrol/formats/customformat )
install ( FILES ${include_nfctag} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/services/nfctag )
install ( FILES ${include_dynlibrary} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/dynlibrary )
install ( FILES ${include_storage} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/services/storage )


set(source ${source_vc} ${source_root} ${source_cards} ${source_crypto} ${source_readerproviders}  ${source_service} ${source_accesscontrol} ${source_encodings} ${source_formats} ${source_customformat} ${source_nfctag} ${source_dynlibrary})

set(include ${include_vc} ${include_root} ${include_cards} ${include_crypto} ${include_readerproviders} ${include_service} ${include_accesscontrol} ${include_encodings} ${include_formats} ${include_customformat} ${include_nfctag} ${include_plugins} ${include_dynlibrary} ${include_storage})

LINK_DIRECTORIES( ${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/${VC_FOLDER_NAME}")

set(LIBLOGICALACCESS_LIBRARY logicalaccess)

add_library(
  logicalaccess
  ${source}
)

install ( TARGETS logicalaccess
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib${LIB_SUFFIX}
          ARCHIVE DESTINATION lib${LIB_SUFFIX}
        )


SET_TARGET_PROPERTIES(logicalaccess PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
target_link_libraries(logicalaccess ${OPENSSL_LIBRARIES})
target_link_libraries(logicalaccess ${ZLIB_LIBRARIES})


if (WIN32)

  set(SCLIB_ENDNAME)
  if (TARGET_ARCH STREQUAL "x64")
	set (SCLIB_ENDNAME "x64")
  endif()

  target_link_libraries(logicalaccess winscard scardsyn${SCLIB_ENDNAME} Winmm ws2_32 delayimp logicalaccess-cryptolib)

elseif (UNIX)
	target_link_libraries(logicalaccess logicalaccess-cryptolib ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
endif()

add_subdirectory(plugins/cryptolib)
add_subdirectory(samples/basic)
add_subdirectory(plugins/pluginscards)
add_subdirectory(plugins/pluginsreaderproviders)

if ( NOT DEFINED LIBLOGICALACCESS_CMAKE_CONFIG_DIR )
  set ( LIBLOGICALACCESS_CMAKE_CONFIG_DIR lib${LIB_SUFFIX}/cmake/liblogicalaccess )
endif ()
set(LIBLOGICALACCESS_DEFINITIONS "${CXX11_FLAGS}")
set ( LIBLOGICALACCESS_INCLUDE_DIRS ${LIBLOGICALACCESS_INCLUDE_DIR} )
list ( APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR} )
list ( APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )
if ( UNIX )
  list (APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${PCSCLITE_INCLUDE_DIRS} )
endif ()
list ( APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} )
list ( REMOVE_DUPLICATES LIBLOGICALACCESS_INCLUDE_DIRS )
set ( LIBLOGICALACCESS_LIBRARIES ${LIBLOGICALACCESS_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} )
set ( LIBLOGICALACCESS_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} )
list ( APPEND LIBLOGICALACCESS_LIBRARIES ${Boost_LIBRARIES} )
list ( APPEND LIBLOGICALACCESS_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} )
set ( LIBLOGICALACCESS_USE_FILE ${CMAKE_INSTALL_PREFIX}/${LIBLOGICALACCESS_CMAKE_CONFIG_DIR}/UseLibLogicalAccess.cmake )
set ( LIBLOGICALACCESS_ROOT_DIR ${CMAKE_INSTALL_PREFIX} )
configure_file ( cmake/LibLogicalAccessConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/LibLogicalAccessConfig.cmake @ONLY )
install ( FILES ${CMAKE_CURRENT_BINARY_DIR}/LibLogicalAccessConfig.cmake cmake/UseLibLogicalAccess.cmake
          DESTINATION ${LIBLOGICALACCESS_CMAKE_CONFIG_DIR}
        )

if (NOT DEFINED SYSCONF_INSTALL_DIR)
  set ( SYSCONF_INSTALL_DIR etc )
endif ()
install ( FILES liblogicalaccess.config DESTINATION ${SYSCONF_INSTALL_DIR} )


