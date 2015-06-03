cmake_minimum_required(VERSION 2.6)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" )
endif()


project(liblogicalaccess)

include(version.cmake)
set(LIBLOGICALACCESS_VERSION_STRING ${LIBLOGICALACCESS_VERSION_MAJOR}.${LIBLOGICALACCESS_VERSION_MINOR}.${LIBLOGICALACCESS_VERSION_PATCH} )

if (MSVC)
	if (NOT TARGET_ARCH)
		set(TARGET_ARCH "Win32")
	endif()

    SET(BOOST_LIBRARYDIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH})
    SET(BOOST_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/include)
    SET(Boost_USE_STATIC_LIBS ON)

    SET(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH})
    SET(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packages/include)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-Wno-deprecated-declarations) #Openssl is deprecated for OS X 10.7
    if (NOT DEFINED CMAKE_MACOSX_RPATH)
     set(CMAKE_MACOSX_RPATH 0)
    endif()
endif()


list ( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

option ( BUILD_SHARED_LIBS "build shared libraries" ON )

find_package(CXX11 REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

find_package(Boost 1.51 REQUIRED COMPONENTS filesystem system date_time chrono thread)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

if (APPLE)
    SET(CMAKE_DL_LIBS ${CMAKE_DL_LIBS} "-framework CoreFoundation")
endif()

if (UNIX)
  find_package(PCSCLite REQUIRED)
  include_directories(${PCSCLITE_INCLUDE_DIRS})
endif()

include_directories(packages/include)

if(MSVC)
    LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/packages/lib/${TARGET_ARCH}")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(VC_FOLDER_NAME "Debug")
        add_definitions(-DDEBUG -D_DEBUG)
    else()
            set(VC_FOLDER_NAME "Release")
    endif()
endif()

add_definitions(-DBOOST_ASIO_NO_IOSTREAM)
if(UNIX)
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /FS")
  endif ()
  if (TARGET_ARCH STREQUAL "Win32")
    add_definitions(-DWIN32 -D_WIN32)
  else()
	add_definitions(-DWIN64 -D_WIN64)
  endif()
  
  SET(VERSION_PRODUCTNAME_VALUE "Liblogicalaccess")
  SET(VERSION_PROJECT_FILE_DESCRIPTION_VALUE "${VERSION_PRODUCTNAME_VALUE}\ (${TARGET_ARCH})")
  SET(LIBLOGICALACCESS_WINDOWS_VERSION ${LIBLOGICALACCESS_VERSION_MAJOR},${LIBLOGICALACCESS_VERSION_MINOR},${LIBLOGICALACCESS_VERSION_PATCH},0 )
  add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}" -DVERSION_PROJECT_FILE_DESCRIPTION_VALUE="${VERSION_PROJECT_FILE_DESCRIPTION_VALUE}" -DVERSION_FILE_AND_PRODUCT_VALUE_STRING="${LIBLOGICALACCESS_VERSION_STRING}" -DVERSION_FILE_AND_PRODUCT_VALUE=${LIBLOGICALACCESS_WINDOWS_VERSION})
  add_definitions(-DLOGICALACCESS_LOGS -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS -DLIBLOGICALACCESS_EXPORTS -DLIBLOGICALACCESS_EXPORTS_DLL)
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

INCLUDE_DIRECTORIES(${NATIVE_INCLUDE} ${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/include)

if(UNIX)

set(LIBLOGICALACCESS_LIBRARY logicalaccess)

file(GLOB source_dynlibrary
${NATIVE_SOURCE}/dynlibrary/posixdynlibrary.cpp
${NATIVE_SOURCE}/dynlibrary/librarymanager.cpp
${NATIVE_SOURCE}/dynlibrary/dllinit.cpp
)
INCLUDE_DIRECTORIES("/usr/local/include")
file(GLOB include_dynlibrary ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/posixdynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/idynlibrary.hpp ${NATIVE_INCLUDE}/logicalaccess/dynlibrary/librarymanager.hpp)

else()

set(LIBLOGICALACCESS_LIBRARY liblogicalaccess)

file(GLOB source_vc ${CMAKE_CURRENT_SOURCE_DIR}/vc++/*.cpp)
file(GLOB resourcerc ${CMAKE_CURRENT_SOURCE_DIR}/vc++/resource.rc)
file(GLOB source_dynlibrary ${NATIVE_SOURCE}/dynlibrary/windowsdynlibrary.cpp ${NATIVE_SOURCE}/dynlibrary/winclass.cpp ${NATIVE_SOURCE}/dynlibrary/librarymanager.cpp)

file(GLOB include_vc ${CMAKE_CURRENT_SOURCE_DIR}/vc++/*.h)
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

SOURCE_GROUP("Header Files\\cards" FILES ${include_cards})
SOURCE_GROUP("Header Files\\dynlibrary" FILES ${include_dynlibrary})
SOURCE_GROUP("Header Files\\readerproviders" FILES ${include_readerproviders})
SOURCE_GROUP("Header Files\\services" FILES ${include_service})
SOURCE_GROUP("Header Files\\services\\accesscontrol" FILES ${include_accesscontrol})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\encodings" FILES ${include_encodings})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats" FILES ${include_formats})
SOURCE_GROUP("Header Files\\services\\accesscontrol\\formats\\customformat" FILES ${include_customformat})
SOURCE_GROUP("Header Files\\services\\nfctag" FILES ${include_nfctag})
SOURCE_GROUP("Header Files\\services\\storage" FILES ${include_storage})

endif()

install ( FILES LICENSE.LibLogicalAccess.txt DESTINATION share/licenses/liblogicalaccess )

set(LIBLOGICALACCESS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

install ( FILES ${include_root} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess )
install ( FILES ${include_cards} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/cards )
install ( FILES ${include_readerproviders} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/readerproviders )
install ( FILES ${include_service} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services )
install ( FILES ${include_accesscontrol} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/accesscontrol )
install ( FILES ${include_encodings} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/accesscontrol/encodings )
install ( FILES ${include_formats} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/accesscontrol/formats )
install ( FILES ${include_customformat} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/accesscontrol/formats/customformat )
install ( FILES ${include_nfctag} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/nfctag )
install ( FILES ${include_dynlibrary} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/dynlibrary )
install ( FILES ${include_storage} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/services/storage )

if (WIN32)
    install ( FILES ${include_vc} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess )
    LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/plugins/cryptolib/${VC_FOLDER_NAME}")
endif()

set(source ${source_root} ${source_cards} ${source_readerproviders}  ${source_service} ${source_accesscontrol} ${source_encodings} ${source_formats} ${source_customformat} ${source_nfctag} ${source_dynlibrary} ${source_vc} ${resourcerc})

set(include ${include_root} ${include_cards} ${include_readerproviders} ${include_service} ${include_accesscontrol} ${include_encodings} ${include_formats} ${include_customformat} ${include_nfctag} ${include_dynlibrary} ${include_storage} ${include_vc})


add_library(
  ${LIBLOGICALACCESS_LIBRARY}
  ${source}
  ${include}
)

install ( TARGETS ${LIBLOGICALACCESS_LIBRARY}
          RUNTIME DESTINATION bin/${LIB_SUFFIX}
          LIBRARY DESTINATION lib/${LIB_SUFFIX}
          ARCHIVE DESTINATION lib/${LIB_SUFFIX}
        )


set_target_properties(${LIBLOGICALACCESS_LIBRARY} PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
target_link_libraries(${LIBLOGICALACCESS_LIBRARY} ${OPENSSL_LIBRARIES})
if (WIN32)
	set_target_properties(${LIBLOGICALACCESS_LIBRARY} PROPERTIES LINK_FLAGS "${LINK_FLAGS} /DELAYLOAD:ssleay32.dll")
endif()


if (WIN32)

  set(SCLIB_ENDNAME)
  if (TARGET_ARCH STREQUAL "x64")
	set (SCLIB_ENDNAME "x64")
  endif()

  target_link_libraries(${LIBLOGICALACCESS_LIBRARY} winscard scardsyn${SCLIB_ENDNAME} Winmm ws2_32 delayimp logicalaccess-cryptolib)

elseif (UNIX)
	target_link_libraries(${LIBLOGICALACCESS_LIBRARY} logicalaccess-cryptolib ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
endif()

add_subdirectory(plugins/cryptolib)
add_subdirectory(plugins/pluginscards)
add_subdirectory(plugins/pluginsreaderproviders)
add_subdirectory(src/tests)

if ( NOT DEFINED LIBLOGICALACCESS_CMAKE_CONFIG_DIR )
  set ( LIBLOGICALACCESS_CMAKE_CONFIG_DIR lib/${LIB_SUFFIX}/cmake/liblogicalaccess )
endif ()
set(LIBLOGICALACCESS_DEFINITIONS "${CXX11_FLAGS}")
set ( LIBLOGICALACCESS_INCLUDE_DIRS ${LIBLOGICALACCESS_INCLUDE_DIR} )
list ( APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR} )
list ( APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )
set(LIBLOGICALACCESS_DEFINITIONS "${LIBLOGICALACCESS_DEFINITIONS} -DLOGICALACCESS_LOGS")
if ( UNIX )
  set(LIBLOGICALACCESS_DEFINITIONS "${LIBLOGICALACCESS_DEFINITIONS} -DUNIX")
  list (APPEND LIBLOGICALACCESS_INCLUDE_DIRS ${PCSCLITE_INCLUDE_DIRS} )
else ()
  set(LIBLOGICALACCESS_DEFINITIONS "${LIBLOGICALACCESS_DEFINITIONS} -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS ")
endif ()
list ( REMOVE_DUPLICATES LIBLOGICALACCESS_INCLUDE_DIRS )
set ( LIBLOGICALACCESS_LIBRARIES ${LIBLOGICALACCESS_LIBRARY} ${OPENSSL_LIBRARIES} )
set ( LIBLOGICALACCESS_LIBRARY_DIRS ${CMAKE_INSTALL_PREFIX}/lib/${LIB_SUFFIX} )
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


